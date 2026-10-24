// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Görev: Model F fabrika preset veri bankasını üretir; üretilen sıra/ad/ayar eşleşmesini koruyun.
const fs=require('fs');
const p=fs.readFileSync('crates/model-f/src/params.rs','utf8');
const defaults={};for(const m of p.matchAll(/^    (\w+): "\w+" => \("[^"]+", [\d.-]+, [\d.-]+, ([\d.-]+),/gm))defaults[m[1]]=Number(m[2]);
const names={
Bass:['Foundation','Rubber Circuit','Sub Authority','Copper Teeth','Concrete Pulse','Night Driver','Acid Foundry','Roundhouse','Seismic','Dusty Ladder','Short Fuse','Black Rubber','Low Orbit','Resin Bass','Pressure Line','Soft Machine','Warehouse','Fifth Floor','Dark Matter','Super Weight'],
Lead:['First Light','Seven Suns','Red Current','Needlepoint','Copper Solo','Signal Flare','Wide Horizon','Afterburner','Glass Razor','Soft Voltage','Mono Chrome','Electric Silk','Fifth Avenue','Liquid Metal','Northern Lead','Hot Circuit','Twin Peaks','Open Sky','Last Transmission','Supernova'],
Key:['Copper Tines','Short Circuit','Velvet Clav','Dust Keys','Ladder Bell','Paper Piano','Pulsewood','Soft Mallet','Neon Chord','Golden Wire','Dawn Keys','Plastic Ivory','Broken Harp','Round Pluck','Static Clav','Small Hours','Octave Glass','Warm Reed','Seven Keys','Muted Brass'],
Pad:['Slow Horizon','Seven Clouds','Warm Static','Dust Halo','Velvet Sky','Frozen Copper','Blue Hour','Soft Focus','Dark Cinema','Open Water','Analog Bloom','Paper Moon','Low Tide','Silver Drift','Distant Choir','Amber Field','Night Glass','Cloud Engine','Deep Sleep','First Snow'],
FX:['Radio Ghost','Metal Rain','Broken Antenna','Steam Valve','Solar Wind','Circuit Birds','Ring Alarm','Filter Siren','Ion Storm','Tape Monster','Alien Current','Pink Eclipse','Pressure Drop','Glass Engine','White Furnace','Falling Satellite','Copper Swarm','Short Wave','Dark Reactor','Last Spark'],
Arp:['Copper Steps','Seven Runner','Night Sequence','Pulse Engine','Little Orbit','Upward Signal','Glass Ladder','Fifth Motion','Trance Wire','Foundry Dance','Low Voltage','Neon Stair','Soft Patterns','Acid Relay','Clockwork','Wide Sequence','Falling Stars','Signal Loop','Super Highway','After Hours']};
const bank=[];
for(const [cat,list]of Object.entries(names))list.forEach((name,i)=>{
 let v={...defaults};let desc='';
 switch(cat){
 case 'Bass':Object.assign(v,{mode:[0,0,0,3,0,1,0,2,0,0,0,4,0,0,3,0,1,3,0,1][i],octave:i%4===0?0:1,waveform:[0,3,2,3,4,2,2,3,0,2,5,3,2,1,4,0,2,2,3,2][i],cutoff:[-1.7,-1.1,-2.2,-.7,-1.5,-.3,-1.2,-1.9,-2.5,-1.3,-.8,-1.8,-2,-.9,-.5,-1.4,-.6,-1,-2.3,-.4][i],emphasis:[1.2,3.1,.8,2.2,1.5,2.4,7.1,.5,.2,4.2,3.8,2.7,.9,3.4,5.1,1.1,2.3,1.6,3,.7][i],contour:[2.5,5.1,1.2,4,3.6,3.8,6.1,2,1.8,3,6.5,2.8,3.2,4.2,5.7,2.1,3.4,2.6,1.3,4.4][i],attack:0,decay:[2.8,3.4,2.3,3.1,2,3.6,2.7,4.2,3.9,2.6,1.8,3.3,2.4,3.7,2.2,4,2.9,3.2,3.5,3.8][i],sustain:i%3===0?6.8:3.2,detune:[4,8,2,7,12,28,3,9,1,14,6,19,5,10,11,16,32,13,3,43][i],glide:i%5===1?1.8:0});desc='Deep analogue bass; play in the lower register.';break;
 case 'Lead':Object.assign(v,{mode:[0,1,0,0,2,0,1,1,4,0,0,1,3,4,1,0,2,1,0,1][i],octave:2,waveform:[2,2,3,5,2,2,2,2,3,0,4,2,2,2,2,3,2,2,1,2][i],cutoff:.3+(i%5)*.42,emphasis:[2.3,1.2,3.4,4.1,1.5,2.8,.7,2.1,3.8,1.8,4.3,2.6,1.1,4.7,1.9,3.2,2.4,.4,3.6,1.6][i],contour:2+(i%4)*.7,attack:i%5===0?.7:0,decay:3+(i%6)*.25,sustain:6.5+(i%4)*.5,detune:[7,52,11,4,13,19,65,78,28,3,16,44,8,31,58,12,10,83,17,72][i],glide:i%3===0?2:0,mod_depth:i%5===0?.6:0,osc_mod:i%5===0?1:0,lfo_rate:4.5});desc='Forward solo voice with sustained body and a bright ladder response.';break;
 case 'Key':Object.assign(v,{mode:[0,0,0,0,4,0,0,0,1,0,0,0,4,0,0,0,2,0,1,3][i],octave:2,waveform:[0,3,4,1,0,0,3,0,2,5,1,3,0,2,4,1,0,3,2,2][i],cutoff:-.8+(i%5)*.42,emphasis:1+(i%6)*.52,contour:3.5+(i%4)*.8,attack:0,decay:2.1+(i%7)*.28,sustain:[.3,1.2,.8,.4,1,.5,1.4,.2,2.6,.7,.9,1.6,.1,.3,1.1,.6,1.8,2.1,2.9,3.5][i],detune:[3,8,5,11,23,2,9,1,34,14,6,10,21,4,13,7,15,12,47,17][i]});desc='Polyphonic key or pluck with a short shared filter/amplifier contour.';break;
 case 'Pad':Object.assign(v,{mode:[1,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0][i],octave:i%5===0?1:2,waveform:[2,2,0,2,2,3,2,1,2,2,3,2,0,2,1,2,4,2,2,0][i],cutoff:-1.2+(i%6)*.36,emphasis:.5+(i%5)*.38,contour:1.1+(i%4)*.5,attack:4.2+(i%6)*.4,decay:5.6+(i%4)*.5,sustain:7.5+(i%3)*.5,detune:[36,68,12,53,45,8,62,14,38,57,17,76,7,49,11,64,19,81,29,5][i],noise:i%4===0?.6:0,noise_color:1,mod_depth:.35+(i%3)*.15,filter_mod:1,lfo_rate:1.2+(i%4)*.6});desc='Slow attack and long contour decay; SuperSaw pads support eight-note polyphony.';break;
 case 'FX':Object.assign(v,{mode:[4,5,4,0,1,4,4,4,1,2,5,0,0,4,0,2,1,4,5,0][i],octave:i%6,waveform:i%6,cutoff:-1.5+(i%7)*.5,emphasis:4.6+(i%6)*.65,contour:2+(i%4)*1.4,attack:i%3===0?3.8:0,decay:3.4+(i%5)*.65,sustain:5+(i%4)*.9,detune:20+(i%8)*9,noise:[2,1,3,5,.5,1.6,0,2.2,4,.4,1,6,3.3,1.4,7,0,2.8,4.3,.7,2.5][i],noise_color:i%2,mod_depth:2+(i%5)*1.1,filter_mod:1,osc_mod:i%3===0?1:0,lfo_rate:2+(i%7)});desc='Held-note sound effect; use the modulation wheel for additional motion.';break;
 case 'Arp':Object.assign(v,{mode:[0,1,0,0,2,0,0,3,1,0,0,0,0,0,4,1,0,2,1,0][i],octave:i%4===0?1:2,waveform:[2,2,3,4,0,2,0,2,2,3,1,5,0,2,3,2,0,4,2,1][i],cutoff:-1+(i%6)*.42,emphasis:1.1+(i%7)*.65,contour:3.3+(i%5)*.65,attack:0,decay:1.6+(i%6)*.2,sustain:.5+(i%4)*.4,detune:[8,48,13,5,11,7,2,9,67,17,6,12,3,14,26,58,4,10,76,15][i],arp_on:1,arp_mode:[0,0,2,0,2,0,1,0,0,3,0,2,3,0,2,1,1,2,0,3][i],arp_rate:[3,3,1,3,2,3,4,1,3,3,1,5,2,3,4,3,1,3,3,2][i],arp_octaves:1+(i%3),arp_gate:.38+(i%5)*.08,tempo:[118,132,108,126,114,128,102,120,138,124,96,134,110,130,122,136,106,116,140,112][i]});desc='Hold a chord; the internal arpeggiator supplies the rhythmic notes.';break;
 }
 if(v.mode===1)v.waveform=2;
 if(cat==='Key'||cat==='Pad')v.mode=v.mode===1?1:0; // Key and Pad use the polyphonic modes.
 bank.push({format:2,name,category:cat,description:desc,values:v});
});
// Forty additional sounds use the multi-stage modulator, preserving the original bank.
const extraNames={
Bass:['Copper Ratchet','Elastic Current','Saw Sculptor','Rubber Steps','Low Voltage Gate','Subterranean Arc'],
Lead:['Vector Solo','Bent Light','Singing Copper','Pulse Lantern','Orbiting Saw','Broken Laser'],
Key:['Glass Mechanism','Circuit Harp','Clockwork Keys','Copper Ripple','Folded Bell','Prism Pluck'],
Pad:['Moving Horizon','Saffron Clouds','Breathing Alloy','Slow Geometry','Amber Tide','Drifting Layers','Northern Circuit','Ribbon Sky'],
FX:['Signal Cartography','Robot Language','Metal Bloom','Fragmented Radio','Shifting Gravity','Wild Circuit'],
Arp:['Copper Staircase','Eightfold Motion','Neon Clock','Ribbon Sequence','Saw Cascade','Pulse Geometry','Amber Steps','Machine Garden']};
for(const [cat,names]of Object.entries(extraNames))names.forEach((name,i)=>{
const source=bank.filter(p=>p.category===cat)[i%20];const v={...source.values};
Object.assign(v,{mseg_enable:1,mseg_duration:[.25,.5,.75,1,1.5,2,3,4][i],mseg_mode:cat==='Pad'?1:(i%3),mseg_amount:cat==='FX'?.08+(i%3)*.025:.08+(i%4)*.035,mseg_target:1,mseg_curve:(i%5-2)*.2});
if(cat==='Pad'){v.mode=1;v.mseg_amount=.12+i*.025;v.mseg_duration=1.5+i*.5;}
if(cat==='Bass'){v.mode=6;v.attack=0;v.mseg_mode=0;v.mseg_duration=.2+i*.075;v.sustain=5;}
if(cat==='Lead'){v.mode=i%2?1:6;if(i===1||i===5){v.mseg_target=2;v.mseg_amount=.025+(i%3)*.015;}}
if(cat==='Key'){v.mode=0;v.mseg_mode=0;v.mseg_duration=.3+i*.1;v.attack=0;}
if(cat==='FX'){v.mode=6;v.mseg_target=[2,2,3,1,2,3][i];v.mseg_amount=[.14,.18,.22,.4,.28,.45][i];v.mseg_mode=1;v.mseg_duration=.35+i*.14;}
if(cat==='Arp'){v.mode=i%2?1:0;v.mseg_mode=i%2;v.mseg_duration=.15+i*.045;v.decay=2+(i%4)*.3;}
if(i%4===3){v.mseg_target=3;v.waveform=3;v.mode=cat==='Bass'?6:0;v.mseg_amount=.3;}
if(v.mseg_target===1){v.cutoff=Math.min(v.cutoff,-1);v.emphasis=Math.min(v.emphasis,4);}
for(let j=0;j<16;j++){v['mseg_p'+j+'_on']=j<6?1:0;if(j<6){v['mseg_p'+j+'_x']=[.1,.25,.42,.6,.78,.92][j];v['mseg_p'+j+'_y']=[[1,.1,.8,.15,.65,.05],[.1,.8,.3,.9,.2,.5],[.8,.6,.4,.2,.7,.1]][i%3][j];}}
for(let j=0;j<18;j++){v['mseg_n'+j+'_in']=j%2?.25:0;v['mseg_n'+j+'_out']=j%2?0:-.2;v['mseg_n'+j+'_wave']=0;v['mseg_n'+j+'_depth']=1;}
if(i%3===2){v.mseg_curve=0;for(let j=0;j<6;j++){v['mseg_n'+j+'_wave']=1+((i+j)%8);v['mseg_n'+j+'_depth']=.18+(j%3)*.1;}}
// Lower output for effects/pitch movement, followed by measured validation.
v.volume=Math.min(v.volume,cat==='FX'?6.5:8.0);
bank.push({format:2,name,category:cat,description:'Multi-stage '+cat.toLowerCase()+' with '+['unused','filter motion','oscillator pitch motion','output level motion'][v.mseg_target]+'.',values:v});
});
const levels=JSON.parse(fs.readFileSync('crates/model-f/sounds/factory-levels.json','utf8'));
for(const p of bank)if(levels[p.name]!==undefined)p.values.volume=levels[p.name];
fs.writeFileSync('crates/model-f/sounds/factory.json',JSON.stringify(bank,null,2)+'\n');
console.log('Wrote',bank.length,'presets with',Object.keys(defaults).length,'controls each.');
