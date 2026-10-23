# Fazıl Irak tarafından oluşturuldu: 23.10.2026
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (c) 2026 Fazıl Irak
"""Package matching sources, original dependencies, notices and Windows binaries."""
from pathlib import Path
import hashlib
import json
import re
import zipfile

ROOT = Path(__file__).resolve().parents[1]
EXCLUDED_IR = 'upstream/reevr/resources/impulses/Hall Quad.flac'
DIRECTORIES = ('.github', 'Source', 'Tests', 'scripts', 'upstream', 'Resources', 'licenses', 'docs', 'doc')
ROOT_FILES = ('CMakeLists.txt', 'README.md', 'AGENTS.md', '.gitignore', '.gitattributes', 'LICENSE',
              'LICENSE-AGPL-3.0.txt', 'COPYING.txt', 'THIRD-PARTY-NOTICES.txt',
              'upstream-lock.json', 'Pitch-dependencies.json')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def package_sources(output, version, source_name=None):
    sources = [ROOT / name for name in ROOT_FILES]
    for folder in DIRECTORIES:
        for path in (ROOT / folder).rglob('*'):
            if not path.is_file():
                continue
            relative = path.relative_to(ROOT)
            if any(part in ('.git', '__pycache__', 'node_modules') for part in relative.parts):
                continue
            if path.suffix in ('.pyc', '.pyo') or relative.as_posix() == EXCLUDED_IR:
                continue
            sources.append(path)
    sources.sort(key=lambda path: path.relative_to(ROOT).as_posix())
    ir_manifest = json.loads((ROOT / 'Resources/impulses/manifest.json').read_text(encoding='utf-8'))
    assert digest(ROOT / ir_manifest['file']) == ir_manifest['sha256'], 'IR manifest mismatch'
    assert (ROOT / 'licenses/Ubuntu-Font-Licence-1.0.txt').stat().st_size > 1000
    source_name = source_name or f'Shaper-{version}-Source.zip'
    with zipfile.ZipFile(output / source_name, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for path in sources:
            archive.write(path, path.relative_to(ROOT).as_posix())
        archive.writestr('SOURCE-MANIFEST.json', json.dumps(
            {path.relative_to(ROOT).as_posix(): digest(path) for path in sources},
            ensure_ascii=False, indent=2) + '\n')
    with zipfile.ZipFile(output / source_name) as archive:
        assert archive.testzip() is None
        assert EXCLUDED_IR not in archive.namelist()
        manifest = json.loads(archive.read('SOURCE-MANIFEST.json'))
        for name, expected in manifest.items():
            assert hashlib.sha256(archive.read(name)).hexdigest() == expected, name

    return source_name, ir_manifest, len(sources)


def package():
    version = re.search(r'project\(Shaper VERSION ([\d.]+)',
                        (ROOT / 'CMakeLists.txt').read_text(encoding='utf-8'))[1]
    output = ROOT / 'dist' / ('Shaper-' + version + '-distribution')
    output.mkdir(parents=True, exist_ok=True)
    log = ROOT / 'build/Testing/Temporary/LastTest.log'
    log_text = log.read_text(encoding='utf-8', errors='replace')
    assert log_text.count('Test Passed.') == 4 and 'Test Failed.' not in log_text, 'Run all four CTest targets successfully first'
    resource_header = (ROOT / 'build/juce_binarydata_reevr_resources/JuceLibraryCode/reevr_data.h').read_text()
    assert 'Shaper_Hall_Quad_wav' in resource_header and 'Hall_Quad_flac' not in resource_header
    assert len(list((ROOT / 'dist/FactoryPresets').rglob('*.shaper'))) == 160
    assert len(list((ROOT / 'dist/EffectPresets').rglob('*.shapereffect'))) == 32
    source_name, ir_manifest, source_count = package_sources(output, version)

    artefacts = ROOT / 'build/Shaper_artefacts/Release'
    executable = artefacts / 'Standalone/Shaper.exe'
    vst = artefacts / 'VST3/Shaper.vst3'
    assert executable.is_file() and vst.is_dir(), 'Build Windows binaries first'
    module_info = (vst / 'Contents/Resources/moduleinfo.json').read_text(encoding='utf-8')
    assert f'"Version": "{version}"' in module_info, 'Binary/source version mismatch'
    ir_bytes = (ROOT / ir_manifest['file']).read_bytes()
    for binary in (executable, vst / 'Contents/x86_64-win/Shaper.vst3'):
        content = binary.read_bytes()
        assert ir_bytes in content, 'Synthetic IR missing from linked binary'
        assert b'Deltaworks' not in content and b'Samplicity' not in content, 'Original IR still embedded'
    log = ROOT / 'build/Testing/Temporary/LastTest.log'
    assert log.is_file(), 'Run CTest before packaging'
    binaries = [(executable, 'Standalone/Shaper.exe')]
    binaries += [(p, 'VST3/Shaper.vst3/' + p.relative_to(vst).as_posix())
                 for p in vst.rglob('*') if p.is_file()]
    for folder in ('FactoryPresets', 'EffectPresets'):
        binaries += [(p, folder + '/' + p.relative_to(ROOT / 'dist' / folder).as_posix())
                     for p in (ROOT / 'dist' / folder).rglob('*') if p.is_file()]
    binaries += [(p, 'VolumePresets/' + p.name)
                 for p in (ROOT / 'Source/Embedded/VolumePresets').iterdir() if p.is_file()]
    binaries += [(ROOT / name, name) for name in ROOT_FILES if name not in ('CMakeLists.txt', 'AGENTS.md', '.gitignore')]
    binaries += [(p, p.relative_to(ROOT).as_posix()) for p in (ROOT / 'licenses').rglob('*') if p.is_file()]
    binaries += [(log, 'test-results.txt')]
    binaries += [(p, p.relative_to(ROOT).as_posix()) for p in (ROOT / 'docs').rglob('*') if p.is_file()]
    binary_name = f'Shaper-{version}-Windows-x64.zip'
    with zipfile.ZipFile(output / binary_name, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for path, name in binaries:
            archive.write(path, name)
        archive.writestr('SOURCE.txt',
                        f'Corresponding source: {source_name}\n'
                        'Distribute both archives together with equivalent download access.\n')
        archive.writestr('BINARY-MANIFEST.json', json.dumps(
            {name: digest(path) for path, name in binaries}, indent=2) + '\n')
    with zipfile.ZipFile(output / binary_name) as archive:
        assert archive.testzip() is None
    release = {'version': version, 'source': source_name, 'binary': binary_name,
               'sha256': {name: digest(output / name) for name in (source_name, binary_name)},
               'excluded_third_party_asset': EXCLUDED_IR,
               'replacement_ir': ir_manifest}
    (output / 'RELEASE-MANIFEST.json').write_text(json.dumps(release, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(output)
    print('Source entries verified:', source_count)
    print('Both ZIP archives verified; original Samplicity IR excluded.')


if __name__ == '__main__':
    package()
