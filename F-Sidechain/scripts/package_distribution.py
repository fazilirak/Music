# Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
# Dosyanın görevi: Ürün derleme, kaynak hazırlama ve dağıtım yardımcıları.
# Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
# Fazıl Irak — 26.10.2026: Test edilen ikililer ve eşleşen tam kaynak paketi.
from pathlib import Path
import sys,json,subprocess,shutil,tempfile,zipfile,tarfile,hashlib,plistlib
root=Path(__file__).resolve().parents[1]
version=json.loads((root/'source-lock.json').read_text())['version']
mac='--macos' in sys.argv
build=root/('build-macos' if mac else 'build')
log=build/'Testing/Temporary/LastTest.log'
assert 'Test Passed.' in log.read_text(errors='replace') and 'Test Failed.' not in log.read_text(errors='replace'),'CTest must pass first'
out=root/'dist'/('macos' if mac else 'windows');out.mkdir(parents=True,exist_ok=True)
source=f'F-Sidechain-{version}-'+('macOS-Source.zip' if mac else 'Source.zip')
subprocess.run([sys.executable,str(root/'scripts/package_source.py'),str(out/source)],check=True)
with zipfile.ZipFile(out/source) as z:
    assert z.testzip() is None
    assert 'Dependencies/JUCE/CMakeLists.txt' in z.namelist()
    assert 'Source/Volume/PluginProcessor.cpp' in z.namelist()
with tempfile.TemporaryDirectory(dir=build) as t:
    stage=Path(t)/f'F-Sidechain-{version}';stage.mkdir()
    art=build/'FSidechain_artefacts/Release'
    bundles=[('VST3/F-Sidechain.vst3','VST3/F-Sidechain.vst3'),('Standalone/'+('F-Sidechain.app' if mac else 'F-Sidechain.exe'),'Standalone/'+('F-Sidechain.app' if mac else 'F-Sidechain.exe'))]
    if mac: bundles.append(('AU/F-Sidechain.component','AU/F-Sidechain.component'))
    for original,destination in bundles:
        p=art/original;assert p.exists(),str(p)
        if mac:
            info=plistlib.loads((p/'Contents/Info.plist').read_bytes());assert info['CFBundleShortVersionString']==version
            subprocess.run(['lipo',str(p/'Contents/MacOS'/info['CFBundleExecutable']),'-verify_arch','arm64','x86_64'],check=True)
            subprocess.run(['codesign','--force','--sign','-','--timestamp=none',str(p)],check=True)
            subprocess.run(['codesign','--verify','--strict',str(p)],check=True)
        q=stage/destination;q.parent.mkdir(parents=True,exist_ok=True)
        if p.is_dir():shutil.copytree(p,q,symlinks=True)
        else:shutil.copy2(p,q)
    for name in ['README.md','LICENSE.txt','THIRD-PARTY-NOTICES.txt','source-lock.json','preview.png','preview-about.png']:shutil.copy2(root/name,stage/name)
    shutil.copytree(root/'licenses',stage/'licenses')
    shutil.copy2(log,stage/'test-results.txt')
    (stage/'SOURCE.txt').write_text('Corresponding complete source: '+source+'\nhttps://github.com/fazilirak/Music/releases/tag/F-Sidechain-v'+version+'\n',encoding='utf-8')
    manifest={p.relative_to(stage).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in stage.rglob('*') if p.is_file()}
    (stage/'BINARY-MANIFEST.json').write_text(json.dumps(manifest,indent=2))
    binary=f'F-Sidechain-{version}-'+('macOS-Universal.tar.gz' if mac else 'Windows-x64.zip')
    if mac:
        (stage/'MACOS.txt').write_text('macOS 11+; Universal arm64 + x86_64. VST3, AU and standalone. Ad-hoc signed; not Developer ID signed or Apple notarised. Separate real DAW and Intel Mac testing remains necessary.\n')
        with tarfile.open(out/binary,'w:gz') as z:z.add(stage,arcname=stage.name)
    else:
        with zipfile.ZipFile(out/binary,'w',zipfile.ZIP_DEFLATED) as z:
            for p in stage.rglob('*'):
                if p.is_file():z.write(p,p.relative_to(stage).as_posix())
        with zipfile.ZipFile(out/binary) as z:assert z.testzip() is None
(out/'RELEASE-MANIFEST.json').write_text(json.dumps({'version':version,'platform':'macOS Universal' if mac else 'Windows x64','tests':'volume_audio_and_state passed','sha256':{n:hashlib.sha256((out/n).read_bytes()).hexdigest() for n in [binary,source]}},indent=2))
print(out)
