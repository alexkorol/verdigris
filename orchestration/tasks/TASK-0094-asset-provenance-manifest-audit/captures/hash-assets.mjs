// TASK-0094 mechanical asset enumerator. Zero dependencies, no network.
// Hashes only files already present in the resource capsule.
import { createHash } from 'node:crypto';
import { readFileSync, readdirSync, statSync } from 'node:fs';
import { join, resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const TASK_DIR = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const ROOT = resolve(join(TASK_DIR, '..', '..', '..'));

function rel(abs) {
  return abs.slice(ROOT.length + 1).split('\\').join('/');
}
function listFiles(dirRel) {
  const out = [];
  const walk = (abs) => {
    for (const e of readdirSync(abs, { withFileTypes: true })) {
      const p = join(abs, e.name);
      if (e.isDirectory()) walk(p);
      else if (!e.name.endsWith('.md')) out.push(rel(p));
    }
  };
  walk(join(ROOT, dirRel));
  return out.sort();
}

const FAMILIES = {
  'native-visual-kit-svg': {
    classification: 'KEEP',
    license_status: 'project-original',
    provenance: [
      'Generated deterministically by orchestration/tasks/TASK-0141-procedural-native-visual-kit/generate-assets.mjs (seeded procedural builder; role table generate-assets.mjs:450-461; svg/manifest writers :480-703)',
      'native/client/assets/generated/visual_kit.h:1 cites this generator and version task0147-gen-2 ("DO NOT EDIT")',
    ],
    license_evidence: [
      'Project-original procedural vector output; generator code and fixed seeds checked in at TASK-0141; no third-party input',
    ],
    build_use:
      'Source vectors consumed at generation time only; geometry is embedded into native/client/assets/generated/visual_kit.h and compiled into the native client (native/client/main.cpp:42 include; drawn main.cpp:1041-1047). Never read at native runtime.',
    notes: '64x64 viewBox per manifest.json generatorVersion task0147-gen-2.',
  },
  'native-visual-kit-manifest': {
    classification: 'KEEP',
    license_status: 'project-original',
    provenance: [
      'Emitted by TASK-0141 generate-assets.mjs manifestText() alongside the SVGs (generate-assets.mjs:663-703); field generatorVersion "task0147-gen-2"',
    ],
    license_evidence: ['Project-generated data file describing the nine procedural roles/motifs and palettes'],
    build_use:
      'Kit documentation + test fixture (orchestration/tasks/TASK-0141 asset-kit.test.mjs). Not loaded by CMake or the native runtime.',
    notes: '',
  },
  'native-visual-kit-header': {
    classification: 'KEEP',
    license_status: 'project-original',
    provenance: [
      'Checked-in generated artifact; visual_kit.h:1 cites TASK-0141 generator version task0147-gen-2',
      'Consumed by native/client/main.cpp:42 (#include "assets/generated/visual_kit.h"); kit symbols drawn at main.cpp:1041-1047',
    ],
    license_evidence: ['Project-generated C++ header embedding procedural geometry/palette floats'],
    build_use:
      'Compile-time embed into the native client executable. native/CMakeLists.txt has no asset copy/install/package steps (checked by rg); this header is the sole asset dependency of the native build.',
    notes: 'Listed because it is the actual packaged form of the native visual kit.',
  },
  'native-triage-captures': {
    classification: 'KEEP',
    license_status: 'project-original',
    provenance: [
      'Native-client screenshot evidence captured during TASK-0035 (orchestration/tasks/TASK-0035-native-exe-triage/REPORT.md:78-82)',
      'Cited as animation/VFX evidence by orchestration/tasks/TASK-0116-animation-vfx-contract-audit/captures/animation-vfx-matrix.json',
    ],
    license_evidence: ['Screenshots rendered by the project native client itself'],
    build_use:
      'Evidence-only captures; referenced by no build, test harness, or runtime path. Excluded from packaging.',
    notes: 'Tracked in git as audit evidence.',
  },
  'wizard-orb-plates': {
    classification: 'UNKNOWN',
    license_status: 'undocumented',
    provenance: [
      'WIZARD prototype orb plates consumed by browser reference src/core/hud/wizard-orb-renderer.js:8-13 (art.png / empty_aligned.jpg / mask_fullres.png / normal_aligned.jpg / pack_aligned.jpg / stone_aligned.jpg)',
      'docs/product/WIZARD_ARCANE_LATTICE_REFERENCE.md:44-53 lists Vessels of Life & Mana as an intended native HUD adapter candidate; docs/product/VERDIGRIS_CONSTITUTION.md:135-139 same',
      '"statue matte generated offline from the black-background plate" per src/core/hud/wizard-orb-renderer.js:4-5; the upstream WIZARD commit/license for these plates is NOT recorded in this repository',
    ],
    license_evidence: [],
    build_use:
      'Browser reference only (vite import). Native candidate for a future life/mana HUD presentation adapter. Packaging successor must obtain owner license decision BEFORE any copy into native assets.',
    notes: 'Provenance stops at the WIZARD prototype inside this capsule; do not infer a license.',
  },
  'wizard-orb-shader': {
    classification: 'UNKNOWN',
    license_status: 'undocumented',
    provenance: [
      'src/assets/shaders/wizard-orb.frag is the browser WIZARD orb fragment shader (referenced with the plates by the orb renderer; plate matte notes at shader line ~543)',
      'Presentation-only candidate boundary documented at docs/product/WIZARD_ARCANE_LATTICE_REFERENCE.md:45 ("the shader does not become game logic")',
    ],
    license_evidence: [],
    build_use:
      'Browser GLSL only; never compiled by native/CMake. Candidate source text for a native port; license undocumented.',
    notes: '',
  },
  'browser-fonts': {
    classification: 'UNKNOWN',
    license_status: 'undocumented',
    provenance: [
      'Declared for browser UI in src/assets/scss/typography/fonts.scss:2-18 (UIFont=Px437_IBM_PS2thin2.ttf, ChatFont=PxPlus_IBM_VGA8.ttf, GameFont=pixelmix.ttf+pixelmix_bold.ttf)',
      'TASK-0093 typography audit already flags them unresolved: orchestration/tasks/TASK-0093-native-typography-contract-audit/FINDINGS.md:73-77 and :232 ("pixelmix/PxPlus_IBM_VGA8/Px437 files - provenance/licensing unresolved")',
    ],
    license_evidence: [],
    build_use:
      'Browser @font-face assets. The native text-contract successor (TASK-0093) requires an explicit font choice; these files are candidates only until a documented license arrives.',
    notes:
      'NEGATIVE CONTROL family: pixelmix.ttf (+bold), PxPlus_IBM_VGA8.ttf, Px437_IBM_PS2thin2.ttf remain non-shippable for native packaging pending documented license.',
  },
  'tiles-dungeon-atlas': {
    classification: 'KEEP',
    license_status: 'public-domain-cc0-attributed',
    provenance: [
      'src/assets/tiles/DCSS-ATTRIBUTION.md: assembled from Dungeon Crawl Stone Soup rltiles (public domain / CC0) plus three project AI-generated mining tiles appended by tools/build_dungeon_atlas.py',
      'Built by tools/build_dungeon_atlas.py:12,180; referenced by server/maps/layers/dungeon.tsx; gid ranges at server/shared/ui.js:8; imported by src/core/client.js:3',
    ],
    license_evidence: [
      'DCSS-ATTRIBUTION.md documents public domain / CC0 status with attribution note; mining additions are project-generated per src/assets/tiles/sources/mining/README.md',
    ],
    build_use:
      'Browser dungeon atlas. Native candidate tileset; the CC0/PD chain is documented so it may be packaged if a product decision adopts it - DCSS-ATTRIBUTION.md must travel with any native copy.',
    notes: 'Contains appended project mining sprites rock-depleted/rock-copper/rock-tin after the DCSS entries.',
  },
  'tiles-mining-sources': {
    classification: 'KEEP',
    license_status: 'project-original',
    provenance: [
      'src/assets/tiles/sources/mining/README.md: 32px sources generated 2026-08-13 via OpenAI built-in image generation, chroma-keyed, palette-reduced, nearest-neighbour downscaled; prompt summary recorded in-repo',
    ],
    license_evidence: ['In-repo generation workflow + date + prompt documentation (tiles/sources/mining/README.md)'],
    build_use: 'Inputs to tools/build_dungeon_atlas.py (appended to dungeon.png). Browser-side; native candidates inherit the dungeon-atlas verdict.',
    notes: '',
  },
  'tiles-legacy-atlases': {
    classification: 'UNKNOWN',
    license_status: 'undocumented',
    provenance: [
      'Legacy Delaford-era tile/UI atlases: terrain.png + objects.png referenced by server/maps/layers/terrain.tsx and objects.tsx, src/core/client.js:1-2, LoginBackdrop.vue:20-21, Stats.vue:200-201',
      'Gid partition documented at server/shared/ui.js:8 ("terrain.png 0..251 | objects.png 252..539 | dungeon.png 540..")',
      'Editor archives (.xcf/.kra) sit beside them with no provenance statement; docs/rebuild/LEGACY_MATRIX.md:9 marks Delaford maps/graphics REFERENCE_ONLY',
    ],
    license_evidence: [],
    build_use:
      'Browser reference rendering only. Delaford firewall (VERDIGRIS_CONSTITUTION.md:175-180) plus LEGACY_MATRIX REFERENCE_ONLY bar native adoption absent an explicit product decision AND a license record that does not exist today.',
    notes: 'NEGATIVE CONTROL family: terrain.png and objects.png remain non-shippable.',
  },
  'skills-icons': {
    classification: 'KEEP',
    license_status: 'project-original',
    provenance: [
      'src/assets/skills/README.md: six 128px WebP production crops from source/verdigris-starter-skills-generated.png (built-in image generator; authored pixel-art brief; quality-92 WebP)',
      'Consumed by src/components/hud/Quickbar.vue:68-82 against icon ids in server/shared/skills/index.js:9-76',
    ],
    license_evidence: ['In-repo generation/provenance statement (skills/README.md)'],
    build_use: 'Browser quickbar icons. Native quickbar candidate; provenance chain complete.',
    notes: 'Source sheet retained under skills/source/.',
  },
  'actors-shipping-atlases': {
    classification: 'KEEP',
    license_status: 'project-original',
    provenance: [
      'docs/actor-art-pipeline.md:8-14: shipping monster/NPC atlases rebuilt from three fixed ImageGen source sheets via tools/build_actor_sheets.py; runtime contract server/shared/actor-graphics.js',
    ],
    license_evidence: ['actor-art-pipeline.md: "every new actor is an original Verdigris identity"; builder + sources checked in'],
    build_use: 'Browser actor billboards. Native billboard candidates once adopted by product decision; provenance complete.',
    notes: '',
  },
  'actors-source-sheets': {
    classification: 'KEEP',
    license_status: 'project-original',
    provenance: ['docs/actor-art-pipeline.md:8-10 names all three source sheets as fixed pipeline inputs'],
    license_evidence: ['Same document asserts original Verdigris identities rebuilt from ImageGen sheets'],
    build_use: 'Rebuild inputs for tools/build_actor_sheets.py; not shipped directly.',
    notes: '',
  },
  'players-production-sheet': {
    classification: 'KEEP',
    license_status: 'project-original',
    provenance: [
      'docs/player-art-pipeline.md:38-62 records full generation provenance (Codex GPT Image 2 run + complete prompt) and master-retention policy',
      'Consumed by src/core/client.js:7, ChroniclesScreen.vue:160, LoginBackdrop.vue:22, Stats.vue:201; frame contract src/core/config/animation.js:7; asserted by tests/unit/player-animation-frames.spec.js:20',
    ],
    license_evidence: ['player-art-pipeline.md "Generation provenance" section'],
    build_use: 'Browser player contact sheet (4x4 of 64px frames). Native Scion billboard candidate; provenance complete.',
    notes: '',
  },
  'players-master': {
    classification: 'KEEP',
    license_status: 'project-original',
    provenance: ['docs/player-art-pipeline.md:9-10,61: checked-in chroma-key master feeding tools/build_player_sheet.py; kept versioned'],
    license_evidence: ['Same generation-provenance section covers the master'],
    build_use: 'Pipeline input only; not shipped.',
    notes: '',
  },
  'players-legacy-human': {
    classification: 'UNKNOWN',
    license_status: 'undocumented',
    provenance: [
      'Original human.png has NO runtime consumers (rg across src/server/tools/tests finds none); docs/player-art-pipeline.md:40 cites it only as the image reference for the human-v2 generation',
      'Legacy Delaford sprite; LEGACY_MATRIX.md:9 marks Delaford graphics REFERENCE_ONLY',
    ],
    license_evidence: [],
    build_use: 'None. Retained as historical/generation reference only.',
    notes: 'NEGATIVE CONTROL named asset: human.png remains non-shippable.',
  },
  'item-atlases-legacy': {
    classification: 'UNKNOWN',
    license_status: 'undocumented',
    provenance: [
      'Five Delaford-era item atlases consumed by browser src/core/client.js:8-12, Stats.vue:202-206, EquipmentSlot.vue:478-486; vessel-art contract tests/unit/vessel-art.spec.js:11',
      'Editor archives under graphics/items/original/ carry only a GIMP explainer README (no provenance/license)',
    ],
    license_evidence: [],
    build_use:
      'Browser equipment/item atlas rendering. Legacy Delaford graphics: REFERENCE_ONLY per LEGACY_MATRIX.md:9; blocked from native packaging without an owner decision AND a license record that does not exist today.',
    notes: 'NEGATIVE CONTROL family: armor/general/jewelry/vessels/weapons.png remain non-shippable.',
  },
  'inventory-art-wizard': {
    classification: 'KEEP',
    license_status: 'project-original',
    provenance: [
      'src/assets/inventory/README.md: copied from the Verdigris inventory prototype (Z:\\Code\\WIZARD\\tools\\rpg_inventory\\assets) at Delaford commit 2dddfea with matching hashes; prototype art is documented ChatGPT Pro image runs driven by ASSET-BRIEF.md/PROMPT.txt, processed locally by art_matte.py/compose_assets.py (batches 7316f87, 56b81ed through reviewed intake 037f485)',
      'UI textures consumed by InventoryGrid.vue:541, EquipmentRagdoll.vue:326, InventoryItemTooltip.vue:192,240, Inventory.vue:243, AuthContainer.vue:189; curated subset mapped by src/core/inventory/item-art.js',
    ],
    license_evidence: ['inventory/README.md asserts project-created AI imagery, not a third-party RPG icon pack, with prototype commit chain'],
    build_use:
      'Browser Brands & Bonds inventory art. Constitution names the inventory as a native integration candidate (VERDIGRIS_CONSTITUTION.md:141-143); provenance chain documented in-repo.',
    notes:
      'README says "119 item finals"; the capsule holds 114 files under items/. Discrepancy flagged for owner intake reconciliation; per-file hashes here are authoritative for what exists today.',
  },
  'audio-menu-music': {
    classification: 'UNKNOWN',
    license_status: 'undocumented',
    provenance: [
      'Looped by src/components/sub/AudioMainMenu.vue:37; 5 MB bundle-bloat flag archived at docs/archive/code-review.md:229',
      'TASK-0117 audio audit recorded the mp3 but did not establish origin (orchestration/tasks/TASK-0117-audio-music-runtime-audit/FINDINGS.md:106,116)',
      'No composer/license record anywhere in the repository',
    ],
    license_evidence: [],
    build_use:
      'Browser auth-shell music only; no native audio target exists in CMake. The native procedural-audio scheduler (TASK-0157) is synth-only and consumes no files.',
    notes: 'NEGATIVE CONTROL named asset: main_menu.mp3 remains non-shippable.',
  },
  favicon: {
    classification: 'UNKNOWN',
    license_status: 'undocumented',
    provenance: [
      'Referenced by index.html:7 as shortcut icon; no provenance/license record in repo; presumed legacy Delaford-era artwork (unverified)',
    ],
    license_evidence: [],
    build_use: 'Browser favicon served from public/. No native relevance today.',
    notes: 'NEGATIVE CONTROL named asset: sword-icon.png remains non-shippable.',
  },
};

function sniff(p, buf) {
  const f = p.toLowerCase();
  if (f.endsWith('.png')) {
    if (buf.length >= 24 && buf.readUInt32BE(12) === 0x49484452)
      return { type: 'image/png', width: buf.readUInt32BE(16), height: buf.readUInt32BE(20) };
    return { type: 'image/png' };
  }
  if (f.endsWith('.jpg') || f.endsWith('.jpeg')) {
    if (buf.length > 4 && buf[0] === 0xff && buf[1] === 0xd8) {
      let o = 2;
      while (o + 9 < buf.length) {
        if (buf[o] !== 0xff) { o++; continue; }
        const m = buf[o + 1];
        if (m >= 0xc0 && m <= 0xcf && m !== 0xc4 && m !== 0xc8 && m !== 0xcc)
          return { type: 'image/jpeg', width: buf.readUInt16BE(o + 7), height: buf.readUInt16BE(o + 5) };
        o += 2 + buf.readUInt16BE(o + 2);
      }
    }
    return { type: 'image/jpeg' };
  }
  if (f.endsWith('.webp') && buf.length >= 30 && buf.toString('ascii', 0, 4) === 'RIFF' && buf.toString('ascii', 8, 12) === 'WEBP') {
    const cc = buf.toString('ascii', 12, 16);
    if (cc === 'VP8X') return { type: 'image/webp', width: 1 + buf.readUIntLE(24, 3), height: 1 + buf.readUIntLE(27, 3) };
    if (cc === 'VP8 ' && buf[23] === 0x9d && buf[24] === 0x01 && buf[25] === 0x2a)
      return { type: 'image/webp', width: buf.readUIntLE(26, 2) & 0x3fff, height: buf.readUIntLE(28, 2) & 0x3fff };
    if (cc === 'VP8L' && buf[20] === 0x2f) {
      const b = buf.readUIntLE(21, 4);
      return { type: 'image/webp', width: (b & 0x3fff) + 1, height: ((b >> 14) & 0x3fff) + 1 };
    }
    return { type: 'image/webp' };
  }
  if (f.endsWith('.svg')) {
    const t = buf.toString('utf8');
    const vb = t.match(/viewBox="\s*([\d.eE+-]+)[ ,]+([\d.eE+-]+)[ ,]+([\d.eE+-]+)[ ,]+([\d.eE+-]+)/);
    return vb ? { type: 'image/svg+xml', width: Number(vb[3]), height: Number(vb[4]) } : { type: 'image/svg+xml' };
  }
  const table = {
    '.ttf': 'font/ttf', '.otf': 'font/otf', '.mp3': 'audio/mpeg', '.wav': 'audio/wav',
    '.xcf': 'image/x-xcf', '.kra': 'application/x-krita', '.json': 'application/json',
    '.h': 'text/x-chdr', '.frag': 'text/x-glsl-fragment', '.md': 'text/markdown',
  };
  for (const ext of Object.keys(table)) if (f.endsWith(ext)) return { type: table[ext] };
  return { type: 'application/octet-stream' };
}

const groups = [
  ['native-visual-kit-manifest', ['native/client/assets/manifest.json']],
  ['native-visual-kit-header', ['native/client/assets/generated/visual_kit.h']],
  ['native-visual-kit-svg', listFiles('native/client/assets/svg')],
  ['native-triage-captures', listFiles('native/client/triage-captures')],
  ['wizard-orb-plates', listFiles('src/assets/orbs/wizard')],
  ['wizard-orb-shader', ['src/assets/shaders/wizard-orb.frag']],
  ['browser-fonts', listFiles('src/assets/fonts')],
  ['tiles-dungeon-atlas', ['src/assets/tiles/dungeon.png']],
  ['tiles-mining-sources', listFiles('src/assets/tiles/sources/mining')],
  ['tiles-legacy-atlases', [
    'src/assets/tiles/terrain.png', 'src/assets/tiles/objects.png',
    'src/assets/tiles/terrain.xcf', 'src/assets/tiles/objects.xcf', 'src/assets/tiles/objects.png.kra',
  ]],
  ['skills-icons', [...listFiles('src/assets/skills').filter((f) => !f.includes('/source/')), 'src/assets/skills/source/verdigris-starter-skills-generated.png']],
  ['actors-shipping-atlases', ['src/assets/graphics/actors/monsters.png', 'src/assets/graphics/actors/npcs.png']],
  ['actors-source-sheets', listFiles('src/assets/graphics/actors/source')],
  ['players-production-sheet', ['src/assets/graphics/actors/players/human-v2.png']],
  ['players-master', ['src/assets/graphics/actors/players/source/human-v2-magenta.png']],
  ['players-legacy-human', ['src/assets/graphics/actors/players/human.png']],
  ['item-atlases-legacy', [
    ...listFiles('src/assets/graphics/items/original').filter((f) => !f.endsWith('.md')),
    'src/assets/graphics/items/armor.png', 'src/assets/graphics/items/general.png',
    'src/assets/graphics/items/jewelry.png', 'src/assets/graphics/items/vessels.png',
    'src/assets/graphics/items/weapons.png',
  ]],
  ['inventory-art-wizard', [
    ...listFiles('src/assets/inventory/items'),
    'src/assets/inventory/divider.png', 'src/assets/inventory/frame_ornate.png', 'src/assets/inventory/slot_texture.png',
  ]],
  ['audio-menu-music', ['src/assets/audio/music/main_menu.mp3']],
  ['favicon', ['public/sword-icon.png']],
];

const seen = new Set();
const assets = [];
for (const [familyId, paths] of groups) {
  const fam = FAMILIES[familyId];
  for (const rp of [...new Set(paths)].sort()) {
    if (seen.has(rp)) throw new Error(`duplicate listing: ${rp}`);
    seen.add(rp);
    const abs = join(ROOT, rp);
    const buf = readFileSync(abs);
    const s = sniff(rp, buf);
    assets.push({
      path: rp,
      family: familyId,
      type: s.type,
      dimensions: s.width != null ? { width: s.width, height: s.height } : null,
      bytes: statSync(abs).size,
      sha256: createHash('sha256').update(buf).digest('hex'),
      provenance_evidence: fam.provenance,
      license_status: fam.license_status,
      license_evidence: fam.license_evidence,
      build_package_use: fam.build_use,
      classification: fam.classification,
      ...(fam.notes ? { notes: fam.notes } : {}),
    });
  }
}

assets.sort((a, b) => (a.path < b.path ? -1 : a.path > b.path ? 1 : 0));
const counts = { total: assets.length, KEEP: 0, UNKNOWN: 0, BLOCKED: 0 };
for (const a of assets) counts[a.classification]++;
if (!counts.UNKNOWN) throw new Error('negative control violated: no UNKNOWN asset');

process.stdout.write(`${JSON.stringify(counts)}\n`);
await import('node:fs').then((fs) =>
  fs.writeFileSync(
    join(TASK_DIR, 'captures', 'assets.json'),
    `${JSON.stringify({
      task: 'TASK-0094-native-asset-provenance-manifest-audit',
      lane: 'ox-pc-bc',
      model: 'openrouter/stealth/ox-alpha',
      base_commit: 'd2423873c577d299b3b39c56024d1d840993c72b',
      method: {
        hash: 'sha256 whole-file via node:crypto',
        dimensions: 'format header parse: PNG IHDR / JPEG SOFn / WebP VP8X|VP8|VP8L / SVG viewBox; null where the format carries no raster size',
        scope_rule:
          'every asset consumed by native presentation today (SVG kit, generated header, manifest, triage captures) plus every WIZARD/browser-reference asset named by docs as a native candidate; files are hashed only as already present in the resource capsule',
        constraints: 'resource capsule read-only; no network downloads; no asset/license/manifest/product-canon changes',
      },
      counts,
      negative_controls_named: [
        'src/assets/fonts/pixelmix.ttf (+ pixelmix_bold.ttf, PxPlus_IBM_VGA8.ttf, Px437_IBM_PS2thin2.ttf) - font licensing undocumented',
        'src/assets/audio/music/main_menu.mp3 - music origin/licensing undocumented',
        'src/assets/tiles/terrain.png + objects.png (+ .xcf/.kra archives) - legacy Delaford graphics, REFERENCE_ONLY, license undocumented',
        'src/assets/graphics/actors/players/human.png - legacy sprite, unused, license undocumented',
        'src/assets/graphics/items/*.png + original/*.xcf - legacy Delaford item art, license undocumented',
        'src/assets/orbs/wizard/* + shaders/wizard-orb.frag - WIZARD prototype plates/shader, upstream license not recorded here',
        'public/sword-icon.png - favicon origin undocumented',
        'external Claude demo plates (22 PNG, ~50 MB, outside capsule) - unhashable; intake doc defers vendoring',
      ],
      external_candidates_not_hashed: [
        {
          label: 'Claude demo billboard plates (22 PNG, ~50 MB)',
          evidence: 'docs/rebuild/CLAUDE_DEMO_ASSET_INTAKE.md:1-65',
          location: 'C:\\Users\\Alex\\Downloads\\claudedemo (outside repository and resource capsule)',
          status: 'UNKNOWN - unhashable in this capsule; vendoring explicitly deferred until provenance/size/packaging approved',
        },
      ],
      assets,
    }, null, 2)}\n`,
  ),
);
console.log('assets.json written');
