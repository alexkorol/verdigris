/** @vitest-environment node */

import { readFileSync } from 'node:fs';
import { fileURLToPath, URL } from 'node:url';
import { describe, expect, it } from 'vitest';

const workflowPath = name => fileURLToPath(new URL(`../../.github/workflows/${name}`, import.meta.url));
const readWorkflow = name => readFileSync(workflowPath(name), 'utf8');

const triggerBlock = source => source.slice(source.indexOf('on:'), source.indexOf('\npermissions:'));

describe('GitHub Actions signal hygiene', () => {
  it.each(['ci.yml', 'native.yml'])('%s runs pushes and pull requests only on master', (name) => {
    const triggers = triggerBlock(readWorkflow(name));

    expect(triggers).toMatch(/push:\s*\n\s+branches:\s*\[\s*master\s*\]/);
    expect(triggers).toMatch(/pull_request:\s*\n\s+branches:\s*\[\s*master\s*\]/);
    expect(triggers).not.toMatch(/branches-ignore:/);
  });
});
