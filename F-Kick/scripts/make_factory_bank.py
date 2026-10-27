# Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
# Dosyanın görevi: Ürün derleme, kaynak hazırlama ve dağıtım yardımcıları.
# Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
"""Fazıl Irak · özgün 64/48/16 bankası. Rastgele preset üretimi veya dış sample yok."""
import json
from pathlib import Path
root=Path(__file__).resolve().parent.parent
bank=[]
def add(name,category,recipe,**values):
    bank.append(dict(name=name,category=category,recipe=recipe,values=values))
# İlk yedi sesin temel ayarları önceki sürümle eşleşir; Sampler Init araç menüsüne taşınır.
for name,r,s,l,h,d in [('Init - Solid',50,48,500,0,0),('Deep House',48,38,420,.12,2),('Short Punch',56,58,210,.1,3),('Long 808',43,24,1800,.35,4),('Soft Analog',52,32,380,.18,1),('Hard Edge',48,66,600,.65,14),('Rounded High Kick',72,20,650,.25,2)]:
    add(name,'General','Legacy body',root=r,sweep=s,length=l,harmonics=h,drive=d,output=-6 if d>5 else -3)
add('Sub Foundation','General','Sub',root=38,sweep=19,length=720,harmonics=.04,cutoff=1600,attack=2)
# Her satır ayrı gövde / pitch / süre / harmonik / filtre / drive / kontur tasarımıdır.
families={
'Analog':[
('Round Pocket',49,28,310,.08,6500,1,0),('Dry Eight',54,19,180,.12,4200,2,1),('Warm Tape',46,31,440,.22,2800,3,2),('Low Bloom',40,22,640,.06,1900,1,3),('Tight Nine',58,44,235,.32,11000,4,4),('Rounded Club',51,36,370,.2,8500,2,5),('Soft Felt',55,12,260,.09,1400,0,6),('Open Drum',64,23,520,.27,4800,1,7)],
'House':[
('Garage Knock',55,49,225,.23,12500,3,4),('Disco Cushion',59,27,340,.16,7200,2,0),('Deep Pillow',42,24,520,.06,2100,2,3),('Jack Snap',61,56,190,.35,15000,4,1),('Soul Floor',50,32,405,.19,4700,1,6),('Club Anchor',47,46,355,.3,9800,5,5),('Minimal Dot',57,38,135,.13,5700,1,2),('Filtered Loft',53,34,285,.38,1800,3,7)],
'Punch':[
('Front Row',63,57,160,.28,17000,4,1),('Chest Hit',68,40,210,.42,8500,3,5),('Dry Concrete',54,62,245,.5,12000,6,4),('Short Thud',48,25,125,.18,2600,2,2),('Click Focus',60,72,195,.12,19000,2,1),('Low Impact',44,51,270,.27,6500,4,0),('Mid Knock',78,32,180,.36,4700,2,6),('Peak Cut',58,47,300,.4,9800,5,7)],
'Sub':[
('Sine Drop',36,18,950,.01,1400,0,3),('808 Round',41,29,1250,.08,2400,1,0),('Short Sub',39,32,320,.1,2100,1,2),('Long Pillow',33,14,1650,.04,1200,2,3),('Clean Bass',45,22,680,.02,3100,0,6),('Velvet Low',37,26,860,.16,1800,2,5),('Club Sub',48,37,460,.2,4200,3,4),('Soft Landing',43,16,560,.07,1600,1,7)],
'Warm':[
('Dusty Room',60,16,330,.32,2200,3,6),('Felt Beater',65,9,245,.14,1700,1,2),('Muted Skin',72,12,185,.26,2900,2,7),('Wooden Body',69,21,390,.44,3400,2,0),('Amber Drum',56,24,480,.35,4100,4,3),('Small Room',76,17,220,.3,5200,1,1),('Soft Paper',62,13,295,.17,2100,0,5),('Vintage Weight',47,30,570,.29,2600,3,4)],
'Dance':[
('Trance Point',53,68,265,.3,18000,5,1),('Pop Solid',57,41,295,.24,11000,2,0),('Electro Bite',62,64,230,.54,15000,6,4),('Mainstage',49,60,390,.4,12500,7,5),('Breakbeat',70,45,175,.39,9800,4,2),('Fast Floor',58,54,205,.26,14000,3,7),('Bounce Body',52,35,450,.3,7400,4,3),('Bright Anchor',66,52,310,.45,17000,3,6)],
'Studio':[
('Dry Reference',50,26,280,.04,15000,0,0),('Low Reference',42,20,410,.05,4000,0,2),('Medium Room',61,29,360,.2,5500,1,3),('Snappy Skin',74,38,155,.3,9500,1,1),('Mellow Round',55,17,430,.11,2600,1,6),('Firm Center',58,42,255,.25,7500,2,4),('Long Body',46,28,780,.18,3500,1,5),('Clean Finish',64,31,320,.15,6500,0,7)]}
for family,rows in families.items():
    for name,r,s,l,h,c,d,contour in rows:
        add(family+' - '+name,'General',family,root=r,sweep=s,length=l,harmonics=h,cutoff=c,drive=d,output=-6 if d>=5 else -4,bodyGain=-6,attack=1.8 if family=='Warm' else .6,hp=18,**{'contour':contour})
assert len(bank)==64
# Önceki dört techno sesinin ayarları korunur; yeni bankada 65–68 konumları.
for i,name in enumerate(['Deep Rumble','Hard Echo','Wobble','Industrial Tail']):
    v=dict(bodyGain=-5,root=48 if i==1 else 45,sweep=40,length=290,harmonics=.2,hp=22,rumbleMix=.7,rumbleDecay=1.8,rumbleHP=35,rumbleLP=500,rumbleDrive=16,duck=.86,echoMix=.12,output=-6)
    if i==1:v.update(driveType=1,drive=9,rumbleDrive=18,rumbleLP=1100,echoMix=.28,echoDivision=2,echoFeedback=.46,ad_drive=.3)
    if i==2:v.update(lfo_rumbleLP=.6,lfo_rumbleMix=.16,rumbleLP=600,lfoShape=1,ad_rumbleDrive=.2)
    if i==3:v.update(driveType=2,rumbleMix=.8,rumbleDecay=.65,echoMix=.22,echoSource=1,ad_rumbleMix=.35,adDelay=35,adAttack=65,adDecay=420)
    add('Techno - '+name,'Techno','Legacy techno',**v)
techno={
'Rumble':[( 'Basement',42,26,230,.62,1.4,300,10,.92,170),('Tunnel',46,34,270,.78,2.4,460,17,.95,220),('Deep Chamber',38,22,340,.72,3.1,210,12,.89,280),('Short Cellar',50,45,185,.5,.65,650,14,.9,95),('Warehouse',44,41,250,.85,2.8,900,20,.96,200),('Dark Floor',40,29,310,.68,1.8,350,16,.94,150)],
'Hard': [('Concrete',48,58,210,.38,.8,1200,19,.93,115),('Steel Toe',53,68,175,.5,1.2,1500,24,.96,100),('Crusher',43,52,280,.65,1.5,850,26,.94,160),('Impact Wall',58,72,155,.42,.5,1900,22,.98,85),('Heavy Press',46,48,320,.7,1.9,650,20,.95,180),('Dry Hammer',55,61,195,.22,.4,2100,17,.9,80)],
'Acid': [('Corrosion',49,54,240,.5,1.3,1000,18,.94,130),('Resonant Bite',54,64,185,.45,.9,1400,20,.96,100),('Squelch Bed',45,43,290,.68,1.7,720,22,.92,190),('Rust Pulse',51,58,215,.58,1.2,1700,24,.95,140),('Sharp Vessel',60,70,165,.35,.6,2200,16,.97,90),('Low Acid',41,36,350,.75,2,500,20,.94,230)],
'Echo': [('Offbeat Vault',44,36,225,.55,1.3,550,15,.94,190),('Dotted Chamber',49,46,240,.5,1.6,900,18,.95,160),('Dub Floor',40,26,310,.65,2.2,380,12,.9,260),('Metal Steps',53,60,190,.4,.9,1700,23,.96,110),('Half Shadow',46,39,280,.7,1.8,660,16,.93,220),('Low Taps',38,23,370,.6,2.6,260,14,.92,290)],
'Industrial': [('Iron Hall',47,58,250,.68,1.5,1300,25,.96,160),('Scrap Press',55,73,180,.5,.7,2400,28,.97,100),('Furnace',42,44,320,.8,2.1,800,27,.94,210),('Grinding Floor',50,65,225,.7,1.1,1900,26,.95,140),('Iron Dust',58,70,165,.35,.5,2600,24,.98,85),('Concrete Cavern',39,37,380,.85,2.9,480,23,.93,280)],
'Hypnotic': [('Rolling Bed',45,32,260,.65,1.6,550,14,.94,170),('Slow Breather',41,28,340,.72,2.5,340,16,.92,260),('Filter Wheel',49,44,220,.58,1.3,1100,18,.96,145),('Sub Cycle',37,20,410,.68,2.7,260,12,.95,290),('Moving Tunnel',47,38,280,.8,2,720,20,.94,210),('Triplet Shadow',52,49,195,.5,.95,1500,19,.97,120)],
'Minimal': [('Dry Needle',56,52,140,.16,.35,900,8,.96,70),('Small Reactor',51,43,175,.3,.5,650,10,.95,90),('Sub Pin',43,28,215,.38,.7,320,12,.94,110),('Micro Room',62,58,125,.2,.4,1700,11,.98,65),('Tight Pump',48,35,200,.42,.8,480,14,.97,100),('Low Tick',40,24,260,.32,1,240,9,.94,140)],
'Schranz': [('Driving Wall',48,68,215,.58,.7,1700,26,.97,100),('Redline',52,75,175,.64,.9,2300,28,.98,90)]}
for family,rows in techno.items():
    for j,(name,r,s,l,m,dec,lp,dr,duck,rec) in enumerate(rows):
        v=dict(root=r,sweep=s,length=l,bodyGain=-7,harmonics=.3 if family in ('Hard','Industrial','Schranz') else .16,hp=24,rumbleMix=m,rumbleDecay=dec,rumbleHP=30 if r<43 else 42,rumbleLP=lp,rumbleDrive=dr,duck=duck,recover=rec,drive=7 if family in ('Hard','Industrial','Schranz') else 2,driveType=1 if family in ('Hard','Schranz') else 2 if family=='Industrial' else 0,output=-9,contour=(j+2)%8)
        if family=='Acid':v.update(resonance=1.45,cutoff=2400+j*650,ad_cutoff=.55,ad_drive=.22,adDecay=70+j*30,echoMix=.13,echoDivision=2)
        if family=='Echo':v.update(echoMix=.24+j*.025,echoDivision=[1,2,3,0,4,1][j],echoSource=j%2,echoFeedback=.28+j*.045,echoLP=900+j*330)
        if family=='Hypnotic':v.update(lfo_rumbleLP=.28+j*.05,lfo_rumbleMix=.08,lfoShape=[0,1,2,5,0,1][j],lfoDivision=[2,4,1,3,2,0][j],lfoRetrigger=0,ad_rumbleDrive=.12)
        if family in ('Industrial','Schranz'):v.update(echoMix=.13,echoSource=1,echoFeedback=.3,ad_drive=-.22,adDecay=60+j*25)
        add(family+' - '+name,'Techno',family,**v)
assert len(bank)==112
experiments=[
('Rubber Rise',dict(root=42,sweep=18,length=430,lfoShape=5,lfo_pitch=.4,lfoDivision=2,ad_rumbleMix=.4,contour=5)),
('Folded Bell',dict(root=68,sweep=36,length=470,driveType=2,drive=17,harmonics=.9,rumbleMix=.3,rumbleLP=2300,contour=7)),
('Double Heart',dict(root=48,sweep=44,length=480,contour=8,echoMix=.32,echoSync=0,echoTime=170,duck=.5)),
('Reverse Gravity',dict(root=39,sweep=30,length=650,contour=9,ad_pitch=.5,adAttack=260,adDecay=300)),
('Broken Spring',dict(root=57,sweep=62,length=320,lfoShape=3,lfoSync=0,lfoHz=13,lfo_pitch=.18,driveType=1,drive=9,contour=4)),
('Random Foundry',dict(root=44,sweep=48,length=390,lfoShape=4,lfoSync=0,lfoHz=17,lfo_rumbleLP=.8,rumbleMix=.7,driveType=2,rumbleDrive=26,contour=1)),
('Acid Balloon',dict(root=52,sweep=50,length=520,resonance=1.9,cutoff=900,ad_cutoff=.9,adAttack=85,adDecay=430,drive=10,contour=5)),
('Ghost Train',dict(root=35,sweep=22,length=280,echoSource=1,echoMix=.55,echoDivision=2,echoFeedback=.65,rumbleMix=.65,rumbleDecay=2.6,rumbleLP=370,contour=2)),
('Staircase',dict(root=61,sweep=72,length=340,contour=10,drive=6,echoMix=.2,echoDivision=0)),
('Wobble Orbit',dict(root=46,sweep=40,length=580,lfoShape=1,lfoDivision=0,lfo_hp=.35,hp=80,lfo_rumbleLP=.7,rumbleMix=.65,contour=3)),
('Folded Submarine',dict(root=31,sweep=19,length=900,driveType=2,drive=20,cutoff=1800,rumbleMix=.4,rumbleLP=260,contour=0)),
('Laser Heel',dict(root=75,sweep=84,length=150,contour=1,driveType=1,drive=13,echoMix=.3,echoSync=0,echoTime=53,echoFeedback=.42)),
('Delayed Bloom',dict(root=42,sweep=31,length=460,rumbleMix=.25,ad_rumbleMix=.7,adDelay=130,adAttack=110,adDecay=650,rumbleDecay=2.4,contour=6)),
('Falling Circuit',dict(root=58,sweep=57,length=420,lfoShape=5,lfoDivision=1,lfo_drive=.6,drive=13,echoMix=.22,contour=4)),
('Updraft',dict(root=47,sweep=38,length=540,lfoShape=2,lfoDivision=2,lfo_cutoff=.65,cutoff=2600,rumbleMix=.5,contour=9)),
('Pitch Tornado',dict(root=50,sweep=68,length=700,lfoShape=0,lfoSync=0,lfoHz=9,lfo_pitch=.3,ad_pitch=-.25,adDecay=580,driveType=2,drive=11,rumbleMix=.45,contour=10))]
for name,v in experiments:
    add('X - '+name,'Experimental','Creative routing',**(dict(bodyGain=-8,output=-10,hp=22,rumbleDrive=17,duck=.9)|v))
assert len(bank)==128
for p in bank:
    if p['category']=='Techno':p['values']['output']=-12
    if p['category']=='Experimental':p['values']['output']=-12
# Üç vuruşlu velocity 127 ölçümünden elde edilen, inceleme sonrası sabitlenen çıkış kalibrasyonu.
level_file=root/'Resources/FactoryLevels.json'
if level_file.exists():
    levels=json.loads(level_file.read_text(encoding='utf-8'))
    for p in bank:
        if p['name'] in levels:p['values']['output']=levels[p['name']]
for p in bank:
    p['values']['punch']=.8 if p['category']=='Techno' else .45 if p['recipe'] in ('Warm','Studio') else .65
    p['values']['rumbleGain']=12
    p['values']['tracking']=0
    p['values']['output']-=6 # İlk ölçümde taşma olmasın; 0.5 kalibrasyonu daha sonra uygulanır.
rumble_levels=root/'Resources/RumbleLevels.json'
if rumble_levels.exists():
    balances=json.loads(rumble_levels.read_text(encoding='utf-8'))
    for p in bank:
        if p['name'] in balances:p['values']['rumbleGain']=balances[p['name']]
impact_levels=root/'Resources/ImpactLevels.json'
if impact_levels.exists():
    for p in bank:
        entry=json.loads(impact_levels.read_text(encoding='utf-8')).get(p['name'])
        if entry is not None:p['values']['output']=entry
out=root/'Resources/FactoryBank.json';out.write_text(json.dumps(bank,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
# JSON insan tarafından düzenlenebilir ana kaynak; derleme header'ı I/O olmadan aynı veriyi sunar.
lines=['// SPDX-License-Identifier: AGPL-3.0-or-later','// Fazıl Irak · scripts/make_factory_bank.py ile üretilir.','#pragma once','#include <juce_core/juce_core.h>','namespace fkick {','inline const juce::var& factoryBank() { static const auto bank=juce::JSON::parse(R"FKICK(']
lines += [json.dumps(bank,ensure_ascii=False),')FKICK"); return bank; }','}']
(root/'Source/FactoryBank.h').write_text('\n'.join(lines)+'\n',encoding='utf-8')
