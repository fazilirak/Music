# Fazıl Irak tarafından oluşturuldu: 23.10.2026
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (c) 2026 Fazıl Irak
"""Verify universal Mach-O bundles and package matching source and licences."""
from pathlib import Path
import json
import platform
import plistlib
import re
import shutil
import subprocess
import tarfile
import tempfile
from package_distribution import ROOT, ROOT_FILES, digest, package_sources


def package():
    if platform.system() != 'Darwin':
        raise SystemExit('macOS is required to verify and package the real Mac binaries.')
    version = re.search(r'project\(Shaper VERSION ([\d.]+)',
                        (ROOT / 'CMakeLists.txt').read_text(encoding='utf-8'))[1]
    build = ROOT / 'build-macos'
    log = build / 'Testing/Temporary/LastTest.log'
    results = log.read_text(encoding='utf-8', errors='replace')
    if results.count('Test Passed.') != 4 or 'Test Failed.' in results:
        raise RuntimeError('All four CTest targets must pass before packaging.')
    header = (build / 'juce_binarydata_reevr_resources/JuceLibraryCode/reevr_data.h').read_text()
    if 'Shaper_Hall_Quad_wav' not in header or 'Hall_Quad_flac' in header:
        raise RuntimeError('Expected independently generated Shaper IR resource.')
    artefacts = build / 'Shaper_artefacts/Release'
    bundles = [(artefacts / 'Standalone/Shaper.app', 'Standalone/Shaper.app'),
               (artefacts / 'VST3/Shaper.vst3', 'VST3/Shaper.vst3'),
               (artefacts / 'AU/Shaper.component', 'AU/Shaper.component')]
    ir = (ROOT / 'Resources/impulses/Shaper_Hall_Quad.wav').read_bytes()
    for bundle, _ in bundles:
        info = plistlib.loads((bundle / 'Contents/Info.plist').read_bytes())
        if info['CFBundleShortVersionString'] != version:
            raise RuntimeError('Bundle/source version mismatch: ' + str(bundle))
        executable = bundle / 'Contents/MacOS' / info['CFBundleExecutable']
        subprocess.run(['lipo', '-verify_arch', 'arm64', 'x86_64', str(executable)], check=True)
        content = executable.read_bytes()
        if ir not in content or b'Deltaworks' in content or b'Samplicity' in content:
            raise RuntimeError('Incorrect IR in linked Mach-O: ' + str(executable))
        # Apple Silicon requires a code signature; this is a local ad-hoc signature,
        # not a Developer ID signature or Apple notarisation.
        subprocess.run(['codesign', '--force', '--sign', '-', '--timestamp=none', str(bundle)], check=True)
        subprocess.run(['codesign', '--verify', '--strict', '--verbose=2', str(bundle)], check=True)
    for folder, extension, count in [('FactoryPresets', '*.shaper', 160), ('EffectPresets', '*.shapereffect', 32)]:
        if len(list((ROOT / 'dist' / folder).rglob(extension))) != count:
            raise RuntimeError('Missing exported preset bank: ' + folder)
    output = ROOT / 'dist' / f'Shaper-{version}-macOS-distribution'
    output.mkdir(parents=True, exist_ok=True)
    source_name, ir_manifest, source_count = package_sources(output, version, f'Shaper-{version}-macOS-Source.zip')
    binary_name = f'Shaper-{version}-macOS-Universal.tar.gz'
    with tempfile.TemporaryDirectory(prefix='package-', dir=build) as temporary:
        stage = Path(temporary) / f'Shaper-{version}'
        stage.mkdir()
        for bundle, destination in bundles:
            shutil.copytree(bundle, stage / destination, symlinks=True)
        for folder in ('FactoryPresets', 'EffectPresets'):
            shutil.copytree(ROOT / 'dist' / folder, stage / folder)
        shutil.copytree(ROOT / 'Source/Embedded/VolumePresets', stage / 'VolumePresets')
        for name in ROOT_FILES:
            if name not in ('CMakeLists.txt', 'AGENTS.md', '.gitignore'):
                shutil.copy2(ROOT / name, stage / name)
        for folder in ('licenses', 'docs'):
            shutil.copytree(ROOT / folder, stage / folder)
        shutil.copy2(log, stage / 'test-results.txt')
        (stage / 'SOURCE.txt').write_text(f'Corresponding source: {source_name}\n'
                                         'Distribute both archives with equivalent download access.\n', encoding='utf-8')
        (stage / 'MACOS.txt').write_text(
            'Universal arm64 + x86_64. Deployment target: macOS 11.0.\n'
            'Standalone: Standalone/Shaper.app\n'
            'VST3: copy VST3/Shaper.vst3 to ~/Library/Audio/Plug-Ins/VST3/\n'
            'AU: copy AU/Shaper.component to ~/Library/Audio/Plug-Ins/Components/\n'
            'Ad-hoc signed; not Developer ID signed or Apple notarised.\n', encoding='utf-8')
        manifest = {p.relative_to(stage).as_posix(): digest(p) for p in stage.rglob('*') if p.is_file()}
        (stage / 'BINARY-MANIFEST.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
        # tar preserves executable permission bits and bundle symlinks.
        with tarfile.open(output / binary_name, 'w:gz') as archive:
            archive.add(stage, arcname=stage.name)
    with tarfile.open(output / binary_name) as archive:
        for name, expected in manifest.items():
            member = archive.extractfile(f'Shaper-{version}/{name}')
            import hashlib
            if hashlib.sha256(member.read()).hexdigest() != expected:
                raise RuntimeError('Archive verification failed: ' + name)
    release = {'version': version, 'platform': 'macOS', 'architectures': ['arm64', 'x86_64'],
               'deployment_target': '11.0', 'signing': 'ad-hoc', 'notarised': False,
               'tests': '4/4 passed on the build host', 'source_entries': source_count,
               'replacement_ir': ir_manifest,
               'sha256': {name: digest(output / name) for name in (source_name, binary_name)}}
    (output / 'RELEASE-MANIFEST.json').write_text(json.dumps(release, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(output)


if __name__ == '__main__':
    package()
