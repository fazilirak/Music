# Fazıl Irak tarafından oluşturuldu: 23.10.2026
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Başarılı testten sonra eşleşen ikili/kaynak ve lisansları paketler."""
from pathlib import Path
import argparse
import hashlib
import json
import platform
import plistlib
import re
import shutil
import subprocess
import tarfile
import tempfile
import zipfile
from bootstrap_juce import ROOT, verify_tree
from bootstrap_fonts import verify as verify_fonts

ROOT_FILES = ['CMakeLists.txt', 'README.md', 'LICENSE', 'JUCE-LICENSE.md',
              'THIRD-PARTY-NOTICES.txt', 'GELISTIRME-TR.md', 'REVIEW-TR.md',
              '.gitignore', '.gitattributes']
SOURCE_DIRS = ['Source', 'Tests', 'Resources', 'Research', 'docs', 'doc', 'scripts', 'vendor']


def sha(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def package(target):
    if platform.system() != ('Darwin' if target == 'macOS' else 'Windows'):
        raise RuntimeError('Package must be verified on its target operating system')
    version = re.search(r'project\(FSaturator VERSION ([\d.]+)', (ROOT / 'CMakeLists.txt').read_text())[1]
    build = ROOT / ('build-macos' if target == 'macOS' else 'build')
    log = build / 'Testing/Temporary/LastTest.log'
    result = log.read_text(encoding='utf-8', errors='replace')
    if result.count('Test Passed.') != 1 or 'Test Failed.' in result:
        raise RuntimeError('DSP/preset/UI regression test must pass before packaging')
    dependency = verify_tree()
    verify_fonts()
    presets = list((ROOT / 'dist/FactoryPresets').glob('*.irakpreset'))
    if len(presets) != 16 or len(list((ROOT / 'dist/Auditions').glob('*.wav'))) != 6:
        raise RuntimeError('Expected sixteen presets and six audition files')
    artefacts = build / 'IRAK_artefacts/Release'
    if target == 'macOS':
        bundles = [('Standalone/F Saturator.app', 'Standalone/F Saturator.app'),
                   ('VST3/F Saturator.vst3', 'VST3/F Saturator.vst3'),
                   ('AU/F Saturator.component', 'AU/F Saturator.component')]
        for source, _ in bundles:
            bundle = artefacts / source
            info = plistlib.loads((bundle / 'Contents/Info.plist').read_bytes())
            if info['CFBundleShortVersionString'] != version:
                raise RuntimeError('Bundle/source version mismatch')
            executable = bundle / 'Contents/MacOS' / info['CFBundleExecutable']
            subprocess.run(['lipo', str(executable), '-verify_arch', 'arm64', 'x86_64'], check=True)
            subprocess.run(['codesign', '--force', '--sign', '-', '--timestamp=none', str(bundle)], check=True)
            subprocess.run(['codesign', '--verify', '--strict', '--verbose=2', str(bundle)], check=True)
    else:
        bundles = [('Standalone/F Saturator.exe', 'Standalone/F Saturator.exe'),
                   ('VST3/F Saturator.vst3', 'VST3/F Saturator.vst3')]
        exe = artefacts / bundles[0][0]
        command = "$v=(Get-Item -LiteralPath $env:FSATURATOR_BINARY).VersionInfo.ProductVersion; Write-Output $v"
        import os
        env = dict(os.environ, FSATURATOR_BINARY=str(exe))
        actual = subprocess.check_output(['powershell', '-NoProfile', '-Command', command], env=env, text=True).strip()
        if actual.split('+')[0] != version:
            raise RuntimeError('Windows binary/source version mismatch: ' + actual)
    output = ROOT / 'dist' / f'F-Saturator-{version}-{target}-distribution'
    output.mkdir(parents=True, exist_ok=True)
    source_name = f'F-Saturator-{version}-' + ('macOS-Source.zip' if target == 'macOS' else 'Source.zip')
    sources = [ROOT / name for name in ROOT_FILES]
    for folder in SOURCE_DIRS:
        sources.extend(p for p in (ROOT / folder).rglob('*') if p.is_file()
                       and '__pycache__' not in p.parts and p.suffix != '.pyc')
    source_manifest = {p.relative_to(ROOT).as_posix(): sha(p) for p in sources}
    with zipfile.ZipFile(output / source_name, 'w', zipfile.ZIP_DEFLATED) as archive:
        for file in sources:
            archive.write(file, f'F-Saturator-{version}/' + file.relative_to(ROOT).as_posix())
        archive.writestr(f'F-Saturator-{version}/SOURCE-MANIFEST.json', json.dumps(source_manifest, indent=2))
    with zipfile.ZipFile(output / source_name) as archive:
        for name, digest in source_manifest.items():
            if hashlib.sha256(archive.read(f'F-Saturator-{version}/' + name)).hexdigest() != digest:
                raise RuntimeError('Source archive verification failed: ' + name)
    binary_name = f'F-Saturator-{version}-' + ('macOS-Universal.tar.gz' if target == 'macOS' else 'Windows-x64.zip')
    with tempfile.TemporaryDirectory(prefix='fsaturator-', dir=build) as temporary:
        stage = Path(temporary) / f'F-Saturator-{version}'
        stage.mkdir()
        for source, destination in bundles:
            source = artefacts / source
            if source.is_dir():
                shutil.copytree(source, stage / destination, symlinks=True)
            else:
                (stage / destination).parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, stage / destination)
        for folder in ['FactoryPresets', 'Auditions']:
            shutil.copytree(ROOT / 'dist' / folder, stage / folder)
        for name in ['DSP-REPORT.md', 'F-Saturator-preview.png', 'F-Saturator-small-preview.png']:
            shutil.copy2(ROOT / 'dist' / name, stage / name)
        for name in ['README.md', 'LICENSE', 'JUCE-LICENSE.md', 'THIRD-PARTY-NOTICES.txt']:
            shutil.copy2(ROOT / name, stage / name)
        shutil.copytree(ROOT / 'Resources/fonts', stage / 'Font-Licences')
        # JUCE'nin bağımlılık telif/lisans metinleri ikili belgelerinde de korunur.
        juce = ROOT / 'vendor/JUCE'
        explicit = {'modules/juce_graphics/image_formats/jpglib/README',
                    'modules/juce_core/zip/zlib/README',
                    'modules/juce_opengl/opengl/juce_gl.h',
                    'modules/juce_box2d/box2d/Box2D.h'}
        for file in juce.rglob('*'):
            if file.is_file() and (any(word in file.name.lower() for word in ('licen', 'copying', 'notice'))
                                   or file.relative_to(juce).as_posix() in explicit):
                target_file = stage / 'JUCE-Third-Party-Licences' / file.relative_to(juce)
                target_file.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(file, target_file)
        shutil.copytree(ROOT / 'docs', stage / 'docs')
        shutil.copy2(log, stage / 'test-results.txt')
        (stage / 'SOURCE.txt').write_text(f'Corresponding source: {source_name}\n'
            'Full matching JUCE sources and build instructions are included in that archive.\n', encoding='utf-8')
        (stage / 'INSTALL.txt').write_text(
            'macOS: VST3 -> ~/Library/Audio/Plug-Ins/VST3/; AU -> ~/Library/Audio/Plug-Ins/Components/\n'
            'Ad-hoc signed; not Developer ID signed or Apple notarised.\n' if target == 'macOS' else
            'Windows: copy the full VST3 bundle to your shared VST3 folder; open Standalone/F Saturator.exe for standalone use.\n', encoding='utf-8')
        binary_manifest = {p.relative_to(stage).as_posix(): sha(p) for p in stage.rglob('*') if p.is_file()}
        (stage / 'BINARY-MANIFEST.json').write_text(json.dumps(binary_manifest, indent=2), encoding='utf-8')
        if target == 'macOS':
            with tarfile.open(output / binary_name, 'w:gz') as archive:
                archive.add(stage, arcname=stage.name)
            with tarfile.open(output / binary_name) as archive:
                for name, digest in binary_manifest.items():
                    if hashlib.sha256(archive.extractfile(stage.name + '/' + name).read()).hexdigest() != digest:
                        raise RuntimeError('Mac archive verification failed: ' + name)
        else:
            with zipfile.ZipFile(output / binary_name, 'w', zipfile.ZIP_DEFLATED) as archive:
                for file in stage.rglob('*'):
                    if file.is_file():
                        archive.write(file, stage.name + '/' + file.relative_to(stage).as_posix())
            with zipfile.ZipFile(output / binary_name) as archive:
                for name, digest in binary_manifest.items():
                    if hashlib.sha256(archive.read(stage.name + '/' + name)).hexdigest() != digest:
                        raise RuntimeError('Windows archive verification failed: ' + name)
    manifest = {'version': version, 'platform': target, 'tests': 'irak_dsp passed',
                'juce_commit': dependency['commit'], 'source_files': len(sources),
                'signing': 'ad-hoc, not notarised' if target == 'macOS' else 'unsigned',
                'sha256': {name: sha(output / name) for name in [source_name, binary_name]}}
    (output / 'RELEASE-MANIFEST.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    print(output)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--platform', choices=['Windows', 'macOS'], required=True)
    package(parser.parse_args().platform)
