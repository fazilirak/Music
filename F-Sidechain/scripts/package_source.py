"""Package exact application and JUCE dependency source without build caches."""
from pathlib import Path
import sys
import zipfile

root = Path(__file__).resolve().parents[1]
dependency = root / 'Dependencies/JUCE'
if not dependency.is_dir():
    dependency = root.parent / '_Dependencies/JUCE'
if not (dependency / 'CMakeLists.txt').is_file():
    raise SystemExit('Complete JUCE source is required for the source archive')
items = ['Source', 'Resources', 'Tests', 'scripts', 'CMakeLists.txt', 'README.md',
         'THIRD-PARTY-NOTICES.txt', 'LICENSE.txt', 'source-lock.json', 'licenses']
with zipfile.ZipFile(sys.argv[1], 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
    for name in items:
        path = root / name
        paths = sorted(path.rglob('*')) if path.is_dir() else [path]
        for file in paths:
            if file.is_file() and '__pycache__' not in file.parts:
                archive.write(file, file.relative_to(root).as_posix())
    for file in sorted(dependency.rglob('*')):
        if file.is_file() and '.git' not in file.relative_to(dependency).parts:
            archive.write(file, 'Dependencies/JUCE/' + file.relative_to(dependency).as_posix())
print('Complete source archive:', Path(sys.argv[1]).name)
