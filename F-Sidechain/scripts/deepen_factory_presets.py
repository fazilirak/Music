# Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
# Dosyanın görevi: Ürün derleme, kaynak hazırlama ve dağıtım yardımcıları.
# Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
"""Stretch only factory envelope depth to the visual floor (stored Y=1).
Saved user envelopes are never transformed. Re-running this script is idempotent.
"""
from pathlib import Path
import re
import xml.etree.ElementTree as ET

path = Path(__file__).resolve().parents[1] / 'Source/Embedded/VolumePresetData.h'
source = path.read_text(encoding='utf-8-sig')
count = 0

def transform(match):
    global count
    xml_text = match.group(1)
    root = ET.fromstring(xml_text)
    key = 'pattern' + str(int(root.get('currpattern', '1')) - 1)
    values = root.get(key).split()
    assert len(values) % 4 == 0
    ys = [float(values[i]) for i in range(1, len(values), 4)]
    top, bottom = min(ys), max(ys)
    assert bottom > top
    for i in range(1, len(values), 4):
        value = float(values[i])
        value = top + (value - top) * (1.0 - top) / (bottom - top)
        values[i] = format(min(1.0, max(0.0, value)), '.12g')
    xml_text = re.sub(r'\b' + key + r'="[^"]*"', key + '="' + ' '.join(values) + ' "', xml_text)
    xml_text = re.sub(r'\btriggerChn="[^"]*"', 'triggerChn="-1"', xml_text)
    count += 1
    print(f'Volume {count}: bottom {bottom:.6f} -> 1.000000; top {top:.6f} unchanged')
    return 'R"VOLPRESET(' + xml_text + ')VOLPRESET"'

source = re.sub(r'R"VOLPRESET\((.*?)\)VOLPRESET"', transform, source, flags=re.S)
assert count == 8
path.write_text(source, encoding='utf-8')
