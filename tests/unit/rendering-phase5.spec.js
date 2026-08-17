import { readFileSync } from 'node:fs';
import { fileURLToPath, URL } from 'node:url';
import { describe, expect, it } from 'vitest';
import {
  TERRAIN_CONTEXT_OPTIONS,
} from '../../src/core/rendering/terrain-renderer.js';

const readSource = relativePath => readFileSync(
  fileURLToPath(new URL(`../../${relativePath}`, import.meta.url)),
  'utf8',
);

describe('2.5D renderer Phase-5 hardening', () => {
  it('does not retain the WebGL framebuffer after the immediate 2D copy', () => {
    expect(TERRAIN_CONTEXT_OPTIONS).toEqual({
      alpha: true,
      antialias: true,
      preserveDrawingBuffer: false,
    });
  });

  it('keeps context recovery and teardown idempotent', () => {
    const source = readSource('src/core/rendering/terrain-renderer.js');
    expect(source).toContain('this.contextLost = true;');
    expect(source).toContain('this.contextLost = false;');
    expect(source).toContain('if (this.destroyed) {');
    expect(source).toContain('this.canvas.removeEventListener(\'webglcontextlost\'');
    expect(source).toContain('this.gl.deleteProgram(this.program)');
  });

  it('caches the sky gradient by viewport, horizon, and colour', () => {
    const source = readSource('src/core/rendering/perspective-renderer.js');
    expect(source).toContain('this.skyGradientKey');
    expect(source).toContain('if (gradientKey !== this.skyGradientKey)');
    expect(source).toContain('this.skyGradient = null;');
  });
});
