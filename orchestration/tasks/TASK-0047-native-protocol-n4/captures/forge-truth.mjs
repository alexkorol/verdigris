import { createVesselBlock } from '../../../../server/core/items/vesselforge/adapter.js';

const seededRng = (seed) => {
  let state = Math.floor(seed) >>> 0;
  return () => {
    state = (state + 0x6D2B79F5) >>> 0;
    let value = state;
    value = Math.imul(value ^ (value >>> 15), value | 1);
    value ^= value + Math.imul(value ^ (value >>> 7), value | 61);
    return ((value ^ (value >>> 14)) >>> 0) / 4294967296;
  };
};

const show = (label, base, opts) => {
  const v = createVesselBlock(base, opts);
  console.log(label, JSON.stringify({
    material: v.material, form: v.form, displayName: v.displayName,
    ilvl: v.item.ilvl, vessel: v.item.vessel, patience: v.item.patienceMax,
    brands: v.item.brands.map(b => ({ modId: b.modId, tier: b.tier, value: b.value })),
    modifiers: v.combat?.modifiers || null,
    ratings: v.combat?.ratings || null,
    brandLines: v.lines.filter((l) => l.section === 'brand').map((l) => l.text),
  }));
};

show('vessel-ring seed4 ilvl40', { id: 'vessel-ring', name: 'Ring', vesselforge: { formId: 'ring' } }, { rng: seededRng(4), ilvl: 40 });
show('vessel-khopesh seed1670 ilvl40', { id: 'vessel-khopesh', name: 'Khopesh', vesselforge: { formId: 'khopesh' } }, { rng: seededRng(1670), ilvl: 40 });
show('bronze-pike seed1 ilvl20', { id: 'bronze-pike', name: 'Bronze Pike', vesselForm: 'spear', vesselMaterial: 'bronze' }, { rng: seededRng(1), ilvl: 20 });
