const {execFileSync}=require('node:child_process');
const assert=require('node:assert/strict');
const path=require('node:path');
const E=require('../content/wizard/expedition.js');
const exe=process.argv[2]||path.join(__dirname,'../build/cartography_tests.exe');
const actual=execFileSync(exe,['--fingerprints'],{encoding:'utf8'}).trim().split(/\r?\n/);
const expected=[];
for(const recipe of Object.keys(E.RECIPES))for(let size=0;size<3;size++)for(let seed=0;seed<100;seed++){
  const shape=[{columns:4,rows:3,branches:0,loops:0},{columns:6,rows:4,branches:5,loops:2},{columns:10,rows:7,branches:18,loops:8}][size];
  const m=E.generate({recipe,seed,...shape});let hash=2166136261;const mix=v=>{hash=Math.imul(hash^v,16777619)>>>0;};
  m.tiles.forEach(mix);for(const n of m.rooms){[n.cx,n.cy,n.variant,n.rotation,n.depth,n.tier].forEach(mix);for(const s of n.sockets)[s.direction,s.to].forEach(mix);}for(const e of m.expedition.graph.edges)[e.a,e.b].forEach(mix);for(const s of m.spawns)[s.x,s.y,s.count,s.tier].forEach(mix);
  expected.push(`${recipe},${size},${seed},${hash}`);
}
assert.equal(actual.length,expected.length);for(let i=0;i<actual.length;i++)assert.equal(actual[i],expected[i],`Cross-language parity case ${i}`);
console.log(`${actual.length} JS/native parity cases passed: tiles, sockets, topology, room variants, navigation depth and encounter population.`);
