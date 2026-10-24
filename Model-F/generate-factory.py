# Geliştirici: Fazıl Irak | Son düzenleme: 24.10.2026
# Fabrika bankası: mimari, bağımsız zarflar ve müzikal hareket tasarımları.
from pathlib import Path
import re,json
root=Path(__file__).resolve().parent
source=(root/'crates/model-f/src/params.rs').read_text(encoding='utf-8')
defs=re.findall(r'(\w+): "[^"]+" => \("[^"]+",\s*([-\d.]+),\s*([-\d.]+),\s*([-\d.]+),\s*([-\d.]+)\)',source)
defaults={k:float(v) for k,lo,hi,v,step in defs}
assert len(defaults)==172,len(defaults)
path=root/'crates/model-f/sounds/factory.json'

names={
'Bass':['Sub Foundation','Iron Undertow','Pulse Furnace','Locked Piston','FM Root Cell','Ring Cavern','Seven Tonnes','Acid Foundry','Dust Engine','Fifth Gear','Shark Bite','Resonant Drum','Hybrid Pressure','Tunnel Walker','Broken Staircase','Valve Ratchet','Sync Hammer','Metal Footwork','Ring Conveyor','Seven Accents','Acid Labyrinth','Gravel Steps','Fifth Runner','Shark Circuit','Sub Siren','Hybrid Braid'],
'Lead':['Velvet Wire','Copper Anthem','Pulse Needle','Sync Vanguard','FM Glassline','Ring Beacon','Solar Unison','Acid Spear','Rust Signal','Fifth Horizon','Shark Razor','Whistle Chamber','Hybrid Voltage','Velvet Maze','Copper Orbit','Pulse Telegraph','Sync Helix','Glass Stairway','Ring Carousel','Solar Current','Acid Switchback','Rust Transmission','Fifth Spiral','Shark Relay','Whistle Cipher','Voltage Cascade'],
'Key':['Round Felt','Copper Pluck','Reed Cabinet','Sync Clavinet','FM Glass Bell','Ring Mallet','Unison Chime','Pulse Marimba','Dust Piano','Fifth Organ','Shark Harpsichord','Tuned Air','Hybrid Celesta','Felt Clockwork','Copper Pinwheel','Reed Automat','Sync Music Box','Glass Constellation','Ring Steps','Chime Rain','Pulse Lattice','Dust Mosaic','Organ Clock','Shark Filigree','Air Machine','Celesta Labyrinth'],
'Pad':['Quiet Horizon','Copper Veil','Pulse Cathedral','Synced Aurora','FM Icefield','Ring Nebula','Seven Clouds II','Pulse Ocean','Dust Atmosphere','Fifth Sanctuary','Shark Silk','Resonant Mist','Hybrid Monolith','Wide Observatory','Horizon Tides','Copper Canopy','Cathedral Gates','Aurora Motion','Icefield Currents','Nebula Compass','Seven Mirrors','Ocean Drift','Dust Mandala','Sanctuary Choir','Silk Polyrhythm','Mist Topography','Monolith Pulse','Observatory Bloom'],
'FX':['Sub Drop Vessel','Copper Impact','Pulse Alarm','Sync Laser','FM Fracture','Ring Satellite','Seven Swarm','Acid Launch','Dust Collision','Fifth Warning','Shark Burst','Steam Valve','Hybrid Anomaly','Vessel Countdown','Copper Fractal','Alarm Polyrhythm','Laser Matrix','Fracture Codes','Satellite Relay','Swarm Geometry','Acid Coordinates','Collision Chain','Warning Grid','Burst Network','Steam Cipher','Anomaly Engine'],
'Arp':['Sub Transit','Copper Relay','Pulse Sprint','Sync Runner','FM Bell Train','Ring Conveyor II','Seven Locomotive','Acid Framework','Dust Metronome','Fifth Rails','Shark Engine','Whistle Signal','Hybrid Switchyard','Wide Interchange','Transit Polymeter','Copper Labyrinth','Pulse Counterpoint','Sync Braids','Bell Constellation','Ring Euclid','Seven Stations','Acid Triplets','Dust Clockwork','Fifth Crossings','Shark Ratchets','Whistle Semaphore','Switchyard Cipher','Interchange Weave']}
# Her mimari farklı bir üretim biçimidir; ikinci yarı aynı mimariyi MSEG sekansıyla yeniden besteler.
architectures=[
 ('round triangle',0,0,0,0,0.0,0.0,0.0),('saw and sub',2,0,-1,0,0.0,0.0,0.0),
 ('opposed pulses',3,5,1,0,0.0,0.0,0.0),('hard-sync saw',2,2,1,1,0.0,0.0,0.0),
 ('FM triangle carrier',0,0,2,0,0.45,0.0,0.0),('ring product',0,4,1,0,0.0,0.0,0.0),
 ('seven-oscillator unison',2,2,0,0,0.0,0.0,0.0),('narrow pulse pair',4,5,0,0,0.0,0.0,0.0),
 ('noise and reed',0,3,0,0,0.0,1.8,0.0),('fifth saw pair',2,3,0,0,0.0,0.0,0.0),
 ('shark and triangle',1,0,-1,0,0.0,0.0,0.0),('resonant noise',0,0,0,0,0.0,6.0,0.0),
 ('sync/FM hybrid',2,0,1,1,0.32,0.5,0.0),('wide poly duo',2,4,1,0,0.0,0.0,0.0)]
pitch_phrases=[[0,7,3,10,5,12,7,3],[0,3,7,5,10,7,12,5],[0,12,7,3,5,10,3,7],[0,5,10,7,3,12,5,7]]
gates=[[1,.18,.72,0,.92,.28,.55,.06],[.15,1,.35,.78,0,.92,.24,.66],[.9,0,.45,.8,.2,.65,1,.12],[.25,.85,0,1,.4,.15,.7,.95]]
def phrase(v,levels,duration,target,amount,index):
    v.update(mseg_enable=1,mseg_duration=round(duration,3),mseg_mode=1,mseg_target=target,mseg_amount=round(amount,3),mseg_start=round(levels[0],3),mseg_curve=[-.35,0,.28,.55][index%4],mseg_gridx=3,mseg_gridy=4,mseg_snap=0)
    points=[]
    for k in range(1,8):
        points.extend([(k/8-.002,levels[k-1]),(k/8+.002,levels[k])])
    points.extend([(.973,levels[7]),(.992,levels[0])])
    for k,(x,y) in enumerate(points):
        v[f'mseg_p{k}_x']=round(x,4);v[f'mseg_p{k}_y']=round(y,3);v[f'mseg_p{k}_on']=1
    for n in range(18):
        v[f'mseg_n{n}_in']=[-.4,.1,.5][(n+index)%3]
        v[f'mseg_n{n}_out']=[.3,-.25,.65][(n+index)%3]
        v[f'mseg_n{n}_wave']=0
        v[f'mseg_n{n}_depth']=0
    # Filter/volume phrases may add a small arch inside selected held steps.
    if target!=2:
        for n in [1,5,9,13]:v[f'mseg_n{n}_wave']=1;v[f'mseg_n{n}_depth']=.08 if target==1 else .12

bank=[]
for category,labels in names.items():
    half=len(labels)//2
    for i,name in enumerate(labels):
        a=i%half;multi=i>=half
        architecture,w1,w2,octshift,sync,fm,noise,_=architectures[a]
        v=defaults.copy()
        oct1=1 if category=='Bass' else 2
        mode=0 if category in ['Key','Pad'] else 6
        if a==5:mode=5 if category in ['Key','Pad'] else 4
        if a==6:mode=1
        if a==9:mode=3
        v.update(mode=mode,octave=oct1,waveform=w1,osc2_waveform=w2,osc2_octave=max(0,min(5,oct1+octshift)),osc1_level=7.6,osc2_level=6.2,osc1_tune=0,detune=[2,18,0,24,0,38,68,11,7,0,28,0,17,45][a],noise=noise,noise_color=a%2,osc_sync=sync,fm_on=int(fm>0),fm_amount=fm if fm>0 else .2,release=1,overload=[0,1.4,2.8,1.2,0,.8,0,2.4,3.2,0,1.6,0,.5,0][a],lfo_rate=2.1,mod_depth=0,osc_mod=0,filter_mod=0,glide=0,volume=6.4)
        if a==4:v['osc2_level']=0 # muted carrier partner still drives audio-rate FM
        if a==11:v.update(osc1_level=0,osc2_level=0)
        if category=='Bass':
            v.update(cutoff=[-1.1,-.7,-1.4,-.5,.1,-.8,-.9,-1.2,-.5,-.7,-.4,.35,-.2][a],emphasis=[1.5,3.2,4.8,5.2,2.1,4,3.1,7.8,2.6,3.4,6.2,8.6,5.5][a],contour=[2.5,6,4.8,6.2,1,2,3.5,7,4,5,6.5,2,5.8][a],attack=0,decay=[2.2,4,1.5,3.2,4.4,2.6,6,2,3,4.8,1.1,3.4,5][a],sustain=2.5,audio_attack=0,audio_decay=3.2,audio_sustain=8.5,glide=.4 if a in [3,7,12] else 0)
        elif category=='Lead':
            v.update(cutoff=[.1,.7,-.1,1.0,1.5,.6,.5,.15,.8,.35,1.25,1.2,.75][a],emphasis=[1.5,3,5,6,2,4,2.5,7.5,2,3.8,5.4,8.5,6.5][a],contour=3.5,attack=.2,decay=4.6,sustain=5.5,audio_attack=.1,audio_decay=4.3,audio_sustain=8.8,glide=1.3 if a in [1,3,7,12] else 0)
        elif category=='Key':
            v.update(cutoff=[-.3,.3,.8,.5,1.8,.9,.5,-.4,.2,.4,1.1,1.4,.9][a],emphasis=2.2 if a!=11 else 8.3,contour=4.8,attack=0,decay=[4,2.1,4.8,2.8,6.2,4.4,6.5,2.4,3.6,7,1.8,3,5.2][a],sustain=1.0,audio_attack=0,audio_decay=[4.8,2.5,5.2,3,6.6,4.8,6.8,2.8,4,7.8,2.2,3.4,5.7][a],audio_sustain=7 if a==9 else 0,volume=5.7)
        elif category=='Pad':
            v.update(cutoff=[-.8,-.4,.1,.3,.8,.15,-.5,-.2,.2,-.6,.45,1.0,.05,.25][a],emphasis=2.8 if a!=11 else 8.5,contour=3.2,attack=5.2+(a%4)*.7,decay=7.2,sustain=7.4,audio_attack=4.6+(a%4)*.6,audio_decay=7.8,audio_sustain=8.4,mod_depth=.12+(a%3)*.09,filter_mod=1,lfo_rate=.7+(a%4)*.45,volume=5.4)
        elif category=='FX':
            v.update(cutoff=[-1.5,.2,1.4,.8,2.1,.6,-.3,1.1,.3,.8,1.3,1.7,.5][a],emphasis=8.8 if a in [2,3,7,11] else 5.5,contour=7.8,attack=0 if a%3 else 3.2,decay=5.8,sustain=1.2,audio_attack=0 if a%3 else 2.4,audio_decay=6.4,audio_sustain=1.8,mod_depth=1.8 if a in [2,6,10] else 0,osc_mod=int(a in [2,6]),filter_mod=int(a==10),lfo_rate=5.8 if a==2 else 3.5,volume=5.8)
        else:
            v.update(cutoff=[-.6,.2,-.3,.65,1.4,.2,-.4,-.7,.3,-.1,.8,1.2,.45,.25][a],emphasis=6.4 if a in [2,3,7,10] else 3.4,contour=5.8,attack=0,decay=2.6,sustain=2.0,audio_attack=0,audio_decay=3.2,audio_sustain=1.8,arp_on=1,arp_mode=[0,2,1,0,3,2,0,1,3,2,0,3,1,2][a],arp_rate=[3,4,5,2,3,4,3,5,2,3,4,5,3,4][a],arp_octaves=1+a%3,arp_gate=[.42,.64,.28,.72][a%4],arp_sync=0,tempo=128,volume=5.9)
        # AD one-shots include transient pitch falls, filter swells and amplitude pulses.
        if not multi:
            target=2 if a in [4,11] or (category=='FX' and a%3==0) else 1
            amount=(-.38 if category=='FX' else -.0625) if target==2 else [.23,-.26,.48,.34][a%4]
            v.update(mseg_enable=0,mseg_target=target,mseg_amount=amount,mseg_attack=.42 if category=='Pad' else [.0,.18,.36,.08][a%4],mseg_decay=.72 if category=='Pad' else [.28,.48,.65,.36][a%4])
            motion='AD transient pitch contour' if target==2 else 'AD secondary filter contour'
        else:
            target=2 if a%4==0 else 3 if a%4==1 else 1
            if category=='Pad':target=1 if a%3 else 3
            if category=='FX' and a%4==0:target=2
            duration=(5.2+(a%4)*1.1) if category=='Pad' else (1.875 if category=='Arp' else [2,3,4,2.5][a%4])
            levels=[n/12 for n in pitch_phrases[a%4]] if target==2 else gates[a%4]
            amount=.25 if target==2 else (.62 if category=='Pad' and target==3 else 1) if target==3 else [.35,-.42,.58,.3][a%4]
            # Arpeggiator her notada zarfı yeniden tetikler; uzun MSEG cümlesi için tuş basılı tutulur.
            if category=='Arp': v.update(arp_on=0,audio_sustain=7.0,sustain=4.2,contour=2.8,cutoff=v['cutoff']+.5,volume=6.8)
            phrase(v,levels,duration,target,amount,a)
            motion=['','eight-step contour sequence','eight-step pitch phrase','accented rhythmic amplitude sequence'][target]
        description=f'0.2.0 original bank · {"Multi" if multi else "AD"} · {architecture} · independent Filter/Audio contours · {motion}. Hold a note or chord to hear the full phrase.'
        bank.append(dict(format=2,name=name,category=category,description=description,values=v))
assert len(bank)==160
assert sum(p['values']['mseg_enable']>0.5 for p in bank)==80
assert len({json.dumps(p['values'],sort_keys=True) for p in bank})==160
path.write_text(json.dumps(bank,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
(root/'docs/BANK-020.json').write_text(json.dumps([dict(name=p['name'],category=p['category'],description=p['description'],envelope='Multi' if p['values']['mseg_enable'] else 'AD',target=p['values']['mseg_target']) for p in bank],ensure_ascii=False,indent=2),encoding='utf-8')
print('Generated 160 new sounds: 80 AD / 80 Multi, 6 categories, 13/14 structural architectures per category.')
