# Fazıl Irak | 24.10.2026 | GPL-3.0-or-later
"""Mevcut Rust ürünlerini test eder; ikili ve eksiksiz eşleşen kaynaklarını paketler."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import plistlib
import shutil
import subprocess
import sys
import tarfile
import tomllib
import urllib.request
import zipfile

REPO = Path(__file__).resolve().parents[1]
FONT_URL = 'https://codeload.github.com/ArtifexSoftware/urw-base35-fonts/tar.gz/refs/tags/20200910'
FONT_SHA = 'e0d9b7f11885fdfdc4987f06b2aa0565ad2a4af52b22e5ebf79e1a98abd0ae2f'


def run(args, cwd, capture=False):
    print('+', ' '.join(map(str, args)), flush=True)
    return subprocess.run(list(map(str, args)), cwd=cwd, check=True, text=True,
                          stdout=subprocess.PIPE if capture else None).stdout


def sha(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def archive_zip(source, destination, prefix):
    with zipfile.ZipFile(destination, 'w', zipfile.ZIP_DEFLATED, compresslevel=6, strict_timestamps=False) as output:
        for path in sorted(source.rglob('*')):
            if path.is_file():
                output.write(path, (Path(prefix) / path.relative_to(source)).as_posix())


def bundle(destination, binary, identifier, version, app=False):
    executable = destination.stem
    folder = destination / 'Contents/MacOS'
    folder.mkdir(parents=True)
    shutil.copy2(binary, folder / executable)
    (folder / executable).chmod(0o755)
    info = {'CFBundleExecutable': executable, 'CFBundleIdentifier': identifier,
            'CFBundleName': executable, 'CFBundleVersion': version,
            'CFBundleShortVersionString': version, 'CFBundlePackageType': 'APPL' if app else 'BNDL',
            'LSMinimumSystemVersion': '11.0', 'NSHighResolutionCapable': True,
            'NSMicrophoneUsageDescription': 'Audio input is used only for local audio processing.'}
    (destination / 'Contents/Info.plist').write_bytes(plistlib.dumps(info))
    subprocess.run(['codesign', '--force', '--deep', '--sign', '-', str(destination)], check=True)
    subprocess.run(['codesign', '--verify', '--deep', '--strict', str(destination)], check=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('project', choices=['Model-F', 'F-EQ'])
    parser.add_argument('--package-only', action='store_true')
    args = parser.parse_args()
    root = REPO / args.project
    if not root.exists() and (REPO / 'Cargo.toml').exists():
        root = REPO  # Tam kaynak ZIP'i tek proje köküne açılır.
    model = args.project == 'Model-F'
    crate, library, example, display = (('model-f', 'model_f', 'model-f-standalone', 'Model F') if model else
                                       ('simple-eq', 'simple_eq', 'simple-eq', 'F EQ'))
    manifest_path = root / ('crates/model-f/Cargo.toml' if model else 'Cargo.toml')
    version = tomllib.loads(manifest_path.read_text(encoding='utf-8'))['package']['version']
    mac = sys.platform == 'darwin'
    platform = 'macOS' if mac else 'Windows'
    os.environ['MACOSX_DEPLOYMENT_TARGET'] = '11.0'
    os.environ['CARGO_TARGET_DIR'] = str(root / 'target')
    options = ['--remap-path-prefix=' + str(REPO) + '=Music']
    if not mac:
        options += ['-C', 'target-feature=+crt-static']
    os.environ['CARGO_ENCODED_RUSTFLAGS'] = '\x1f'.join(options)
    cargo = ['cargo']
    common = ['-p', crate, '--release', '--locked']
    if not args.package_only:
        run(cargo + ['test'] + common, root)
        if model:
            run(cargo + ['test', '-p', 'model-f-core', '--test', 'supersaw', '--release', '--locked'], root)
        targets = ['aarch64-apple-darwin', 'x86_64-apple-darwin'] if mac else [None]
        for target in targets:
            flags = ['--target', target] if target else []
            run(cargo + ['build'] + common + ['--lib'] + flags, root)
            run(cargo + ['build'] + common + ['--features', 'standalone', '--example', example] + flags, root)

    dist = root / 'release-dist'
    dist.mkdir(exist_ok=True)
    package = dist / f'{args.project}-{version}-{platform}'
    if package.exists():
        raise RuntimeError('Package already exists; inspect before repackaging')
    package.mkdir()
    if mac:
        universal = dist / 'universal'
        universal.mkdir()
        for kind, stem in [('plugin', f'lib{library}.dylib'), ('standalone', f'examples/{example}')]:
            parts = [root / f'target/{arch}-apple-darwin/release' / stem for arch in ('aarch64', 'x86_64')]
            output = universal / kind
            run(['lipo', '-create', *parts, '-output', output], root)
            arches = run(['lipo', '-archs', output], root, capture=True)
            if not all(arch in arches for arch in ('arm64', 'x86_64')):
                raise RuntimeError('Universal architecture check failed')
        for suffix in ('vst3', 'clap'):
            bundle(package / f'{display}.{suffix}', universal / 'plugin',
                   'com.fazilirak.' + args.project.lower() + '.' + suffix, version)
        bundle(package / f'{display}.app', universal / 'standalone',
               'com.fazilirak.' + args.project.lower() + '.standalone', version, app=True)
    else:
        dll = root / f'target/release/{library}.dll'
        vst = package / f'{display}.vst3/Contents/x86_64-win'
        vst.mkdir(parents=True)
        shutil.copy2(dll, vst / f'{display}.vst3')
        shutil.copy2(dll, package / f'{display}.clap')
        shutil.copy2(root / f'target/release/examples/{example}.exe', package / f'{display}.exe')

    for name in ['README.md', 'LICENSE', 'COPYING', 'THIRD-PARTY-NOTICES.txt', 'Luftikus-LICENSE.txt']:
        if (root / name).exists():
            shutil.copy2(root / name, package / name)
    shutil.copytree(root / 'third_party/urw-gothic', package / 'licences/urw-gothic')
    shutil.copy2(root / 'docs/RELEASE-LICENSING.md', package / 'RELEASE-LICENSING.md')
    revision_file = root / 'BUILD-REVISION.json'
    commit = (json.loads(revision_file.read_text(encoding='utf-8'))['commit'] if revision_file.exists()
              else run(['git', 'rev-parse', 'HEAD'], root, capture=True).strip())
    metadata = json.loads(run(cargo + ['metadata', '--locked', '--format-version', '1'], root, capture=True))
    dependencies = sorted([{'name': p['name'], 'version': p['version'], 'licence': p.get('license')}
                           for p in metadata['packages']], key=lambda p: (p['name'], p['version']))
    (package / 'DEPENDENCY-LICENCES.json').write_text(json.dumps(dependencies, indent=2) + '\n', encoding='utf-8')
    # Kaynakları cargo vendor ile kilit dosyasındaki gerçek revizyonlardan toplarız.
    existing_config = root / '.cargo/config.toml'
    already_vendored = (root / 'vendor').exists() and 'vendored-sources' in existing_config.read_text(encoding='utf-8')
    vendor_config = '' if already_vendored else run(cargo + ['vendor', '--locked', '--versioned-dirs', 'vendor'], root, capture=True)
    source = root / '.release-source'
    if source.exists():
        raise RuntimeError('Source staging already exists')
    source.mkdir()
    skip = {'target', 'dist', 'release-dist', '.release-source', '.git', '__pycache__'}
    for path in root.iterdir():
        if path.name in skip:
            continue
        destination = source / path.name
        if path.is_dir():
            shutil.copytree(path, destination, ignore=shutil.ignore_patterns('.git', 'target', '__pycache__'))
        elif path.suffix not in ('.log',):
            shutil.copy2(path, destination)
    config = source / '.cargo/config.toml'
    config.parent.mkdir(exist_ok=True)
    config.write_text((config.read_text(encoding='utf-8') if config.exists() else '') + '\n' + vendor_config,
                      encoding='utf-8')
    (source / 'scripts').mkdir(exist_ok=True)
    shutil.copy2(__file__, source / 'scripts/rust_audio_release.py')
    (source / 'ci').mkdir(exist_ok=True)
    workflow = REPO / '.github/workflows/rust-audio-release.yml'
    if not workflow.exists():
        workflow = root / 'ci/rust-audio-release.yml'
    shutil.copy2(workflow, source / 'ci/rust-audio-release.yml')
    font_archive = source / 'third_party/urw-base35-fonts-20200910.tar.gz'
    existing_fonts = root / 'third_party/urw-base35-fonts-20200910.tar.gz'
    if existing_fonts.exists():
        shutil.copy2(existing_fonts, font_archive)
    else:
        urllib.request.urlretrieve(FONT_URL, font_archive)
    if sha(font_archive) != FONT_SHA:
        raise RuntimeError('URW original source checksum mismatch')
    with tarfile.open(font_archive) as fonts:
        for weight in ('Book', 'Demi'):
            name = f'URWGothic-{weight}.otf'
            embedded = root / 'third_party/urw-gothic' / name
            data = fonts.extractfile('urw-base35-fonts-20200910/fonts/' + name).read()
            if hashlib.sha256(data).hexdigest() != sha(embedded):
                raise RuntimeError('Embedded font differs from declared source')
    info = {'project': args.project, 'version': version, 'commit': commit, 'platform': platform,
            'licence': 'GPL-3.0-or-later with preserved third-party terms', 'font_source_sha256': FONT_SHA}
    (source / 'BUILD-REVISION.json').write_text(json.dumps(info, indent=2) + '\n', encoding='utf-8')
    # Tam kaynak paketi yerel cache'e işaret etmemeli: çevrimdışı çöz ve bütün manifest yollarını denetle.
    closure = json.loads(run(cargo + ['metadata', '--locked', '--offline', '--format-version', '1'], source, capture=True))
    for dependency in closure['packages']:
        path = Path(dependency['manifest_path']).resolve()
        if not path.is_relative_to(source.resolve()):
            raise RuntimeError('Source dependency escapes the distribution: ' + str(path))
    (source / 'OFFLINE-SOURCE-VALIDATION.json').write_text(
        json.dumps({'all_manifests_inside_source': True, 'packages': len(closure['packages'])}, indent=2) + '\n', encoding='utf-8')

    source_zip = dist / f'{args.project}-{version}-{platform}-Source.zip'
    archive_zip(source, source_zip, f'{args.project}-{version}-Source')
    (package / 'SOURCE.txt').write_text(
        f'Complete corresponding source: {source_zip.name}\nCommit: {commit}\n'
        f'https://github.com/fazilirak/Music/releases/tag/{args.project}-v{version}\n', encoding='utf-8')
    if mac:
        binary = dist / f'{args.project}-{version}-macOS-Universal.tar.gz'
        with tarfile.open(binary, 'w:gz') as archive:
            archive.add(package, arcname=package.name)
    else:
        binary = dist / f'{args.project}-{version}-Windows-x64.zip'
        archive_zip(package, binary, package.name)
    info['assets'] = {p.name: {'sha256': sha(p), 'bytes': p.stat().st_size} for p in (source_zip, binary)}
    (dist / f'{args.project}-{platform}-MANIFEST.json').write_text(json.dumps(info, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(info, indent=2))


if __name__ == '__main__':
    main()
