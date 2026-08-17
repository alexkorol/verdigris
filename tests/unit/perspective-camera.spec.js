import { describe, expect, it } from 'vitest';
import PerspectiveCamera, {
  ARPG_CAMERA_PRESET,
  MAX_USER_ZOOM,
} from '../../src/core/rendering/perspective-camera.js';

const makeCamera = (overrides = {}) => {
  const camera = new PerspectiveCamera();
  camera.update({
    width: 1150,
    height: 750,
    x: 1200,
    y: 1600,
    ...overrides,
  });
  return camera;
};

describe('PerspectiveCamera', () => {
  it('pins the camera target to the focus row', () => {
    const camera = makeCamera();
    const projected = camera.project(1200, 1600);

    expect(projected.x).toBeCloseTo(575, 10);
    expect(projected.y).toBeCloseTo(camera.focus, 10);
    expect(projected.scale).toBeCloseTo(camera.zoom, 10);
  });

  it('round-trips ground points through projection and picking', () => {
    const camera = makeCamera();
    const world = { x: 1472.5, y: 1328.25 };
    const projected = camera.project(world.x, world.y);
    const unprojected = camera.unproject(projected.x, projected.y);

    expect(unprojected.x).toBeCloseTo(world.x, 8);
    expect(unprojected.y).toBeCloseTo(world.y, 8);
  });

  it('scales nearer points continuously larger than farther points', () => {
    const camera = makeCamera();
    const far = camera.project(1200, 900);
    const focus = camera.project(1200, 1600);
    const near = camera.project(1200, 2050);

    expect(far.scale).toBeLessThan(focus.scale);
    expect(focus.scale).toBeLessThan(near.scale);
    expect(near.scale / far.scale).toBeGreaterThan(2);
  });

  it('uses the shared height sampler for terrain projection', () => {
    const camera = new PerspectiveCamera({ heightAt: () => 24 });
    camera.update({ width: 1150, height: 750, x: 1200, y: 1600 });
    const ground = camera.project(1200, 1600, 0);
    const terrain = camera.projectTerrain(1200, 1600);

    expect(terrain.y).toBeCloseTo(ground.y - (24 * ground.scale), 10);
  });

  it('matches the WebGL vertex-shader projection numerically', () => {
    const camera = makeCamera();
    const samples = [
      { x: 720, y: 620, height: 0 },
      { x: 1200, y: 1600, height: 24 },
      { x: 1680, y: 1920, height: 8 },
    ];

    samples.forEach((sample) => {
      const javascript = camera.project(sample.x, sample.y, sample.height);
      const shader = camera.projectWithShaderMath(sample.x, sample.y, sample.height);
      expect(shader.x).toBeCloseTo(javascript.x, 10);
      expect(shader.y).toBeCloseTo(javascript.y, 10);
    });
  });

  it('changes depth of field continuously and strengthens it while zoomed in', () => {
    const wide = makeCamera({ userZoom: 0.72 });
    const close = makeCamera({ userZoom: 1.6 });
    const wideDepth = wide.depthToFocus * 1.22;
    const closeDepth = close.depthToFocus * 1.22;

    expect(wide.circleOfConfusion(wide.depthToFocus)).toBe(0);
    expect(close.circleOfConfusion(close.depthToFocus)).toBe(0);
    expect(wide.dofStrength).toBe(0);
    expect(wide.circleOfConfusion(wideDepth)).toBe(0);
    expect(close.circleOfConfusion(closeDepth)).toBeGreaterThan(
      wide.circleOfConfusion(wideDepth),
    );
  });

  // Phase 4 (plan §7): dofStrength<->zoom blend spans exactly the wheel/pinch
  // range, with the zero floor defined against the ARPG base zoom (plan §9),
  // and no step anywhere that could read as a discrete DoF band (§8.3).
  it('blends depth of field smoothly across the wheel zoom range', () => {
    const samples = [];
    for (let zoom = 0.72; zoom <= MAX_USER_ZOOM + 0.0001; zoom += 0.02) {
      samples.push(makeCamera({ userZoom: zoom }).dofStrength);
    }

    // Zero floor at and below the ARPG base; full strength at the ceiling.
    expect(samples[0]).toBe(0);
    expect(samples.at(-1)).toBeCloseTo(ARPG_CAMERA_PRESET.maxDofStrength, 10);

    // Monotonic non-decreasing, bounded slope: continuous, never banded.
    for (let index = 1; index < samples.length; index += 1) {
      const delta = samples[index] - samples[index - 1];
      expect(delta).toBeGreaterThanOrEqual(0);
      expect(delta).toBeLessThan(0.05);
    }
  });

  it('keeps the circle of confusion continuous across depth (no discrete bands)', () => {
    const camera = makeCamera({ userZoom: MAX_USER_ZOOM });
    const cocs = [];
    for (let ratio = 0.4; ratio <= 2.2; ratio += 0.01) {
      cocs.push(camera.circleOfConfusion(camera.depthToFocus * ratio));
    }

    for (let index = 1; index < cocs.length; index += 1) {
      expect(Math.abs(cocs[index] - cocs[index - 1])).toBeLessThan(0.05);
    }
    // Sharp at the focus plane, saturating to full blur well past it.
    expect(Math.min(...cocs)).toBe(0);
    expect(Math.max(...cocs)).toBe(1);
  });

  it('rejects zero-sized startup viewports without producing projection state', () => {
    const camera = new PerspectiveCamera();

    expect(camera.update({ width: 0, height: 0, x: 0, y: 0 })).toBe(false);
    expect(camera.project(0, 0)).toBe(null);
    expect(camera.unproject(0, 0)).toBe(null);
  });
});
