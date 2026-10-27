# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (c) 2026 Fazıl Irak
# Geliştirici: Fazıl Irak - 27.10.2026
# Dosyanın görevi: Test edilmiş Windows ikilisiyle eşleşen eksiksiz kaynak paketini hazırlamak.
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,tempfile,zipfile
parser=argparse.ArgumentParser();parser.add_argument('project');parser.add_argument('--juce');parser.add_argument('--fonts')
args=parser.parse_args();root=Path(args.project).resolve()
lock=json.loads((root/'source-lock.json').read_text(encoding='utf-8-sig'));name=lock['product'];version=lock['version']
build=root/'build';log=build/'Testing/Temporary/LastTest.log'
text=log.read_text(errors='replace');assert 'Test Passed.' in text and 'Test Failed.' not in text,'CTest must pass'
target={'F-Kick':'FKick','F-TransForm':'FTransForm'}[name]
artefacts=build/(target+'_artefacts/Release')
exe=artefacts/('Standalone/'+name+'.exe');bundle=artefacts/('VST3/'+name+'.vst3')
assert exe.is_file() and bundle.is_dir(),'Build both Windows formats first'
if os.name=='nt':
    import subprocess
    for p in [exe,bundle/('Contents/x86_64-win/'+name+'.vst3')]:
        value=subprocess.check_output(['powershell','-NoProfile','-Command',"(Get-Item -LiteralPath '"+str(p).replace("'","''")+"').VersionInfo.ProductVersion"],text=True).strip()
        assert value==version,'Stale binary version: '+str(p)
juce=Path(args.juce).resolve() if args.juce else root/'Dependencies/JUCE'
assert (juce/'CMakeLists.txt').is_file(),'Complete JUCE source required'
fonts=Path(args.fonts).resolve() if args.fonts else root/'vendor/urw-base35-fonts'
if name=='F-TransForm':
    assert (fonts/'fonts/URWGothic-Book.otf').exists(),'Complete font source required'
    for a,b in [('Book.otf','URWGothic-Book.otf'),('Demi.otf','URWGothic-Demi.otf')]:
        assert (root/'Resources/fonts'/a).read_bytes()==(fonts/'fonts'/b).read_bytes(),'Font provenance mismatch'
out=root/'dist/release';out.mkdir(parents=True,exist_ok=True)
def files(base):
    for p in sorted(base.rglob('*')):
        if p.is_file() and not any(x in {'.git','build','build-macos','__pycache__','.vs'} for x in p.relative_to(base).parts):yield p
def put_tree(z,base,prefix):
    for p in files(base):z.write(p,str(Path(prefix)/p.relative_to(base)).replace('\\','/'))
source=out/(name+'-'+version+'-Source.zip')
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED) as z:
    for part in ['Source','Tests','scripts','Resources','licenses','docs']:
        if (root/part).is_dir():put_tree(z,root/part,part)
    for part in ['CMakeLists.txt','AGENTS.md','README.md','AUTHORS.md','LICENSE','LICENSE.txt','JUCE-LICENSE.md','THIRD-PARTY-NOTICES.txt','.gitignore','source-lock.json','preview.png','preview-about.png']:
        if (root/part).is_file():z.write(root/part,part)
    put_tree(z,juce,'Dependencies/JUCE')
    if name=='F-TransForm':
        put_tree(z,fonts,'vendor/urw-base35-fonts')
        marker=fonts.parent/'URW-VERIFIED.json'
        if marker.is_file():z.write(marker,'vendor/URW-VERIFIED.json')
    z.write(log,'validation/CTest-results.txt')
with zipfile.ZipFile(source) as z:
    assert z.testzip() is None
    for p in ['CMakeLists.txt','Dependencies/JUCE/CMakeLists.txt','AUTHORS.md']:assert p in z.namelist(),p
binary=out/(name+'-'+version+'-Windows-x64.zip')
with zipfile.ZipFile(binary,'w',zipfile.ZIP_DEFLATED) as z:
    z.write(exe,'Standalone/'+exe.name);put_tree(z,bundle,'VST3/'+bundle.name)
    for part in ['LICENSE','LICENSE.txt','THIRD-PARTY-NOTICES.txt','AUTHORS.md','README.md','preview.png','preview-about.png']:
        if (root/part).is_file():z.write(root/part,part)
    put_tree(z,root/'licenses','licenses');z.write(log,'TEST-RESULTS.txt')
    z.writestr('SOURCE.txt','Complete corresponding source: '+source.name+'\nhttps://github.com/fazilirak/Music/releases/tag/'+name+'-v'+version+'\n')
manifest={'product':name,'developer':'Fazıl Irak','version':version,'platform':'Windows x64','tests':'CTest passed','sha256':{}}
for p in [source,binary]:
    with p.open('rb') as f:manifest['sha256'][p.name]=hashlib.file_digest(f,'sha256').hexdigest()
(out/'RELEASE-MANIFEST.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
(out/'SHA256SUMS.txt').write_text(''.join(h+'  '+n+'\n' for n,h in manifest['sha256'].items()),encoding='ascii')
print(out)

