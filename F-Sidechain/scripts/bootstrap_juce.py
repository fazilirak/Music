# Fazıl Irak — 26.10.2026: Sabitlenmiş JUCE kaynak sürümünü getirir.
from pathlib import Path
import json,subprocess
root=Path(__file__).resolve().parents[1]
lock=json.loads((root/'source-lock.json').read_text())
target=root/'Dependencies/JUCE'
if target.exists():
    revision=subprocess.check_output(['git','-C',str(target),'rev-parse','HEAD'],text=True).strip()
    assert revision==lock['juceCommit'],'Existing JUCE version differs from source-lock.json'
else:
    target.mkdir(parents=True)
    for args in [['init'],['remote','add','origin',lock['juceRepository']],['fetch','--depth','1','origin',lock['juceCommit']],['checkout','--detach','FETCH_HEAD']]:
        subprocess.run(['git','-C',str(target),*args],check=True)
