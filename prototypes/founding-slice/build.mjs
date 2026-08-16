// Builds index.html by injecting ./assets/*.png into slice.html as data URIs.
// Usage: node build.mjs
import {readFileSync, writeFileSync, readdirSync, statSync} from 'node:fs';
import {join, dirname} from 'node:path';
import {fileURLToPath} from 'node:url';

const root = dirname(fileURLToPath(import.meta.url));
const assetsDir = join(root, 'assets');
const map = {};
for (const f of readdirSync(assetsDir)) {
  if (!f.endsWith('.png') || f === 'contact_sheet.png') continue;
  const slot = f.replace(/\.png$/, '');
  map[slot] = 'data:image/png;base64,' + readFileSync(join(assetsDir, f)).toString('base64');
}
const src = readFileSync(join(root, 'slice.html'), 'utf8');
if (!src.includes('__ASSETS__')) throw new Error('slice.html missing __ASSETS__ placeholder');
const out = src.replace('__ASSETS__', JSON.stringify(map));
const dest = join(root, 'index.html');
writeFileSync(dest, out);
console.log('wrote index.html:', Math.round(statSync(dest).size / 1024), 'KB, slots:', Object.keys(map).join(', '));
