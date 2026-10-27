# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (c) 2026 Fazıl Irak
# Geliştirici: Fazıl Irak - 27.10.2026
# Dosyanın görevi: Yedi Music projesini yerel Linux x64 üzerinde derlemek, test etmek ve kaynaklarıyla paketlemek.
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,struct,subprocess,sys,tarfile,tempfile
REPO=Path(__file__).resolve().parents[1]
PROJECTS=['F-Kick','F-Sidechain','F-TransForm','F-Saturator','F-EQ','Model-F','Shaper']
def run(args,cwd=REPO,capture=False):
    print('+',' '.join(map(str,args)),flush=True)
    return subprocess.run(list(map(str,args)),cwd=cwd,check=True,text=True,stdout=subprocess.PIPE if capture else None).stdout
def sha(p):
    with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def validate_module(p):
    data=p.read_bytes()[:20]
    if data[:5]!=b'\x7fELF\x02' or data[5]!=1 or struct.unpack('<H',data[18:20])[0]!=62:
        raise RuntimeError('Not a little-endian Linux ELF64 x86_64 module: '+str(p))
    symbols=run(['nm','-D','--defined-only',p],capture=True)
    if not re.search(r'\bGetPluginFactory\b',symbols):raise RuntimeError('Missing VST3 factory symbol')
    libraries=run(['ldd',p],capture=True)
    if 'not found' in libraries:raise RuntimeError('Unresolved runtime dependency: '+libraries)
    return libraries
def source_paths(root):
    skip={'build','build-linux','build-macos','dist','release-dist','target','.release-source','.publish'}
    for p in sorted(root.rglob('*')):
        rel=p.relative_to(root)
        if rel.parts[0] in skip or '.git' in rel.parts or '__pycache__' in rel.parts:continue
        if p.is_file() or p.is_symlink():yield p,rel
def main():
    parser=argparse.ArgumentParser();parser.add_argument('project',choices=PROJECTS);args=parser.parse_args()
    if not sys.platform.startswith('linux'):raise RuntimeError('Native Linux runner required')
    root=REPO/args.project;out=REPO/'linux-artifacts'/args.project;out.mkdir(parents=True,exist_ok=True)
    commit=run(['git','rev-parse','HEAD'],capture=True).strip()
    if args.project in ['Model-F','F-EQ']:
        run([sys.executable,REPO/'scripts/rust_audio_linux_release.py',args.project])
        dist=root/'release-dist';package=next(p for p in dist.iterdir() if p.is_dir() and p.name.endswith('-Linux'))
        modules=list(package.glob('*.vst3/Contents/x86_64-linux/*.so'));assert len(modules)==1
        dependencies=validate_module(modules[0])
        manifest=json.loads((dist/(args.project+'-Linux-MANIFEST.json')).read_text())
        version=manifest['version']
        for name in manifest['assets']:shutil.copy2(dist/name,out/name)
    else:
        for script in ['bootstrap_juce.py','bootstrap_fonts.py']:
            if (root/'scripts'/script).is_file():run([sys.executable,root/'scripts'/script],root)
        build=root/'build-linux'
        options=['cmake','-S',root,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_POSITION_INDEPENDENT_CODE=ON']
        if args.project=='Shaper':options+=['-DSHAPER_BUILD_TESTS=ON']
        run(options,root);run(['cmake','--build',build,'--parallel',os.environ.get('MUSIC_BUILD_JOBS','2')],root)
        run(['ctest','--test-dir',build,'--output-on-failure','--timeout','900'],root)
        log=build/'Testing/Temporary/LastTest.log'
        assert 'Test Passed.' in log.read_text(errors='replace') and 'Test Failed.' not in log.read_text(errors='replace')
        version=re.search(r'project\([^)]*?VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)',(root/'CMakeLists.txt').read_text(),re.S)[1]
        bundles=list(build.glob('*_artefacts/Release/VST3/*.vst3'));assert len(bundles)==1,bundles
        bundle=bundles[0];modules=list(bundle.glob('Contents/x86_64-linux/*.so'));assert len(modules)==1,modules
        dependencies=validate_module(modules[0])
        source=out/(args.project+'-'+version+'-Linux-Source.tar.gz')
        with tarfile.open(source,'w:gz') as archive:
            for p,rel in source_paths(root):archive.add(p,arcname=str(Path(args.project+'-'+version+'-Source')/rel),recursive=False)
            archive.add(log,arcname=args.project+'-'+version+'-Source/validation/LINUX-TEST-RESULTS.txt')
        with tempfile.TemporaryDirectory() as temporary:
            stage=Path(temporary)/(args.project+'-'+version+'-Linux-x64');stage.mkdir()
            (stage/'VST3').mkdir();shutil.copytree(bundle,stage/'VST3'/bundle.name,symlinks=True)
            for name in ['LICENSE','LICENSE.txt','COPYING','COPYING.txt','JUCE-LICENSE.md','THIRD-PARTY-NOTICES.txt','AUTHORS.md','README.md','licenses']:
                p=root/name
                if p.is_dir():shutil.copytree(p,stage/name)
                elif p.is_file():shutil.copy2(p,stage/name)
            if (root/'Resources/fonts').is_dir():shutil.copytree(root/'Resources/fonts',stage/'Font-Licences')
            shutil.copy2(log,stage/'TEST-RESULTS.txt')
            (stage/'SOURCE.txt').write_text('Complete corresponding source: '+source.name+'\nCommit: '+commit+'\n',encoding='utf-8')
            (stage/'LINUX.txt').write_text('Linux x86_64, built and tested on Ubuntu 24.04. Copy the complete VST3 bundle to ~/.vst3/ and rescan in a native Linux VST3 host. Windows and macOS bundles are different binaries. Actual DAW/audio-device and other-distribution testing remains separate.\n',encoding='utf-8')
            binary=out/(args.project+'-'+version+'-Linux-x64.tar.gz')
            with tarfile.open(binary,'w:gz') as archive:archive.add(stage,arcname=stage.name)
    (out/(args.project+'-Linux-RUNTIME-LIBRARIES.txt')).write_text(dependencies,encoding='utf-8')
    assets={p.name:sha(p) for p in out.iterdir() if p.is_file()}
    info={'project':args.project,'version':version,'commit':commit,'platform':'Linux x64 / Ubuntu 24.04','tests':'Native build and project tests passed; ELF64 x86_64, VST3 factory symbol and runtime dependencies verified','assets':assets}
    (out/(args.project+'-Linux-RELEASE-MANIFEST.json')).write_text(json.dumps(info,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(info,indent=2),flush=True)
if __name__=='__main__':main()
