import { describe, it, expect } from 'vitest';

import VerdigrisGeometricTree from '../../src/core/passives/verdigris-geometric-tree.js';

const RING_ONE_IDS = new Set(['1,0', '0,1', '-1,1', '-1,0', '0,-1', '1,-1']);

describe('skill tree first-allocation hint', () => {
  it('recommends a starter node when points exist and only the origin is allocated', () => {
    const tree = new VerdigrisGeometricTree({ availablePoints: 2 });
    const hint = tree.recommendFirstAllocation();

    expect(hint).not.toBeNull();
    expect(RING_ONE_IDS.has(hint.nodeId)).toBe(true);
    expect(hint.name).toBeTruthy();
    expect(hint.amount).toBeGreaterThan(0);
    expect(hint.effects.length).toBeGreaterThan(0);

    const node = tree.nodes.get(hint.nodeId);
    expect(node.connections).toContain('0,0');
  });

  it('is deterministic for the same earned points', () => {
    const first = new VerdigrisGeometricTree({ availablePoints: 2 });
    const second = new VerdigrisGeometricTree({ availablePoints: 2 });
    expect(second.recommendFirstAllocation()).toEqual(first.recommendFirstAllocation());
  });

  it('returns null with no unspent points', () => {
    const tree = new VerdigrisGeometricTree({ availablePoints: 0 });
    expect(tree.recommendFirstAllocation()).toBeNull();
  });

  it('returns null once any non-origin node is allocated', () => {
    const tree = new VerdigrisGeometricTree({ availablePoints: 2 });
    const target = [...tree.nodes.values()].find((node) => node.id !== '0,0');
    target.active = true;
    expect(tree.recommendFirstAllocation()).toBeNull();
  });

  it('exposes the hint through toState so the pane stays reactive', () => {
    const tree = new VerdigrisGeometricTree({ availablePoints: 2 });
    expect(tree.toState().firstAllocationHint).toEqual(tree.recommendFirstAllocation());
  });
});
