import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { spawnSync } from "node:child_process";
import { fileURLToPath } from "node:url";

const SENTINEL = path.join(
  path.dirname(fileURLToPath(import.meta.url)),
  "board-sentinel.mjs",
);

function makeBoard(t, files) {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "board-sentinel-"));
  t.after(() => fs.rmSync(dir, { recursive: true, force: true }));
  for (const [rel, content] of Object.entries(files)) {
    const abs = path.join(dir, rel);
    fs.mkdirSync(path.dirname(abs), { recursive: true });
    fs.writeFileSync(abs, content);
  }
  return dir;
}

function runSentinel(repoDir, extraArgs = []) {
  const res = spawnSync(
    process.execPath,
    [SENTINEL, "--repo", repoDir, "--json", ...extraArgs],
    { encoding: "utf8" },
  );
  let json = null;
  assert.doesNotThrow(() => {
    json = JSON.parse(res.stdout);
  }, `sentinel stdout was not JSON: ${res.stdout} ${res.stderr}`);
  return { code: res.status, json, stderr: res.stderr };
}

function spec(id, slug, extra = "") {
  return `---
task: ${id}
title: ${slug}
state: READY
owned_paths: [orchestration/tasks/${id}-${slug.toLowerCase()}/**]
forbidden_paths: [everything else]
---
`;
}

const READY_TABLE = (rows) => `# Run status

## Effective READY — ${rows.length} packets

| Pri | Task | Topology / job | Preferred route | Owner-visible contribution |
|---|---|---|---|---|
${rows.map((r) => `| P0 | ${r} | INDEPENDENT / MECHANICAL | future after current claim | contribution |`).join("\n")}

## Sequenced successors — 0 DRAFT

| Task | Dependency / release |
|---|---|
`;

const HOLD_SECTION = (rows) => `

## HOLD despite historical READY headers

| Task | Release condition | Reason |
|---|---|---|
${rows.map((r) => `| ${r} | condition | reason |`).join("\n")}
`;

function statusFile(state, coordinator = "codex (worker: test)") {
  return `# status

state: ${state}
coordinator: ${coordinator}
started_at: 2026-08-21 12:00:00 -07:00
`;
}

test("integrated historical READY is excluded from effective READY", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(["TASK-0001 integrated-thing"]),
    "orchestration/INTEGRATION_LOG.md":
      "2026-08-16 - TASK-0001 integrated-thing - integrated as abc123.\n",
    [`orchestration/tasks/TASK-0001-integrated-thing/SPEC.md`]: spec(
      "TASK-0001",
      "integrated-thing",
    ),
    [`orchestration/tasks/TASK-0001-integrated-thing/REVIEW.md`]:
      "---\nverdict: ACCEPTED\n---\n",
    [`orchestration/tasks/TASK-0001-integrated-thing/STATUS.md`]:
      statusFile("INTEGRATED"),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "0"]);
  assert.equal(code, 0);
  assert.deepEqual(json.effective_ready, []);
  assert.deepEqual(json.integrated, ["TASK-0001"]);
  assert.equal(json.counts.effective_ready, 0);
});

test("a live READY row counts", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(["TASK-0002 live-thing"]),
    [`orchestration/tasks/TASK-0002-live-thing/SPEC.md`]: spec(
      "TASK-0002",
      "live-thing",
    ),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.equal(code, 0);
  assert.deepEqual(json.effective_ready, ["TASK-0002"]);
});

test("CLAIMED removes a task from READY", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE([
      "TASK-0003 claimed-thing",
      "TASK-0004 open-thing",
    ]),
    [`orchestration/tasks/TASK-0003-claimed-thing/SPEC.md`]: spec(
      "TASK-0003",
      "claimed-thing",
    ),
    [`orchestration/tasks/TASK-0003-claimed-thing/STATUS.md`]:
      statusFile("CLAIMED", "kimi (worker: k1)"),
    [`orchestration/tasks/TASK-0004-open-thing/SPEC.md`]: spec(
      "TASK-0004",
      "open-thing",
    ),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.equal(code, 0);
  assert.deepEqual(json.effective_ready, ["TASK-0004"]);
  assert.equal(json.counts.claimed, 1);
  assert.match(json.claimed[0].coordinator, /kimi/);
});

test("REVIEW_REQUESTED is surfaced", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(["TASK-0005 reviewable"]),
    [`orchestration/tasks/TASK-0005-reviewable/SPEC.md`]: spec(
      "TASK-0005",
      "reviewable",
    ),
    [`orchestration/tasks/TASK-0005-reviewable/STATUS.md`]:
      statusFile("REVIEW_REQUESTED"),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "0"]);
  assert.equal(code, 0);
  assert.deepEqual(json.effective_ready, []);
  assert.equal(json.counts.review_requested, 1);
  assert.deepEqual(json.review_requested.map((r) => r.id), ["TASK-0005"]);
});

test("overlapping owned paths fail", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE([
      "TASK-0006 broad-owner",
      "TASK-0007 narrow-owner",
    ]),
    [`orchestration/tasks/TASK-0006-broad-owner/SPEC.md`]: `---
task: TASK-0006
state: READY
owned_paths:
  - native/**
---
`,
    [`orchestration/tasks/TASK-0007-narrow-owner/SPEC.md`]: `---
task: TASK-0007
state: READY
owned_paths: [native/client/render.cpp]
---
`,
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.equal(json.counts.collisions, 1);
  assert.equal(json.collisions[0].a, "TASK-0006");
  assert.equal(json.collisions[0].b, "TASK-0007");
  assert.notEqual(code, 0);
  assert.equal(json.healthy, false);
});

test("HOLD is excluded even when its immutable SPEC header says READY", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md":
      READY_TABLE(["TASK-0008 live-thing"]) +
      HOLD_SECTION(["TASK-0009 held-thing"]),
    [`orchestration/tasks/TASK-0008-live-thing/SPEC.md`]: spec(
      "TASK-0008",
      "live-thing",
    ),
    [`orchestration/tasks/TASK-0009-held-thing/SPEC.md`]: spec(
      "TASK-0009",
      "held-thing",
    ),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.equal(code, 0);
  assert.deepEqual(json.effective_ready, ["TASK-0008"]);
  assert.deepEqual(json.hold, ["TASK-0009"]);
  assert.equal(json.counts.effective_ready, 1);
});

test("stale claim is named when its implementation was integrated", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(["TASK-0010 stale-claim"]),
    "orchestration/INTEGRATION_LOG.md":
      "2026-08-16 - TASK-0010 stale-claim - integrated as def456.\n",
    [`orchestration/tasks/TASK-0010-stale-claim/SPEC.md`]: spec(
      "TASK-0010",
      "stale-claim",
    ),
    [`orchestration/tasks/TASK-0010-stale-claim/REVIEW.md`]:
      "---\nverdict: ACCEPTED\n---\n",
    [`orchestration/tasks/TASK-0010-stale-claim/STATUS.md`]:
      statusFile("CLAIMED", "deepseek (worker: ds)"),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "0"]);
  assert.equal(code, 0, "stale claims are reported, not fatal");
  assert.equal(json.counts.stale_claims, 1);
  assert.equal(json.stale_claims[0].id, "TASK-0010");
  assert.ok(
    json.stale_claims[0].reasons.some((r) => /INTEGRATION_LOG/.test(r)),
  );
  assert.deepEqual(json.effective_ready, []);
});

test("duplicate task id fails", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE([
      "TASK-0011 doubled",
      "TASK-0011 doubled again",
    ]),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.notEqual(code, 0);
  assert.ok(json.errors.some((e) => e.type === "duplicate_task_id"));
});

test("malformed task state fails", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(["TASK-0012 broken-state"]),
    [`orchestration/tasks/TASK-0012-broken-state/SPEC.md`]: spec(
      "TASK-0012",
      "broken-state",
    ),
    [`orchestration/tasks/TASK-0012-broken-state/STATUS.md`]:
      statusFile("WAT"),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.notEqual(code, 0);
  assert.ok(json.errors.some((e) => e.type === "malformed_status_state"));
});

test("healthy eight-task runway exits zero", (t) => {
  const files = {
    "orchestration/RUN_STATUS.md": READY_TABLE(
      Array.from({ length: 8 }, (_, i) => `TASK-00${20 + i} packet-${i}`),
    ),
  };
  for (let i = 0; i < 8; i++) {
    const id = `TASK-00${20 + i}`;
    files[`orchestration/tasks/${id}-packet-${i}/SPEC.md`] = spec(
      id,
      `packet-${i}`,
    );
  }
  const dir = makeBoard(t, files);
  const { code, json } = runSentinel(dir, ["--min-ready", "8"]);
  assert.equal(code, 0);
  assert.equal(json.queue_floor.satisfied, true);
  assert.equal(json.counts.effective_ready, 8);
});

test("queue floor violation exits non-zero", (t) => {
  const files = {
    "orchestration/RUN_STATUS.md": READY_TABLE(
      Array.from({ length: 8 }, (_, i) => `TASK-00${30 + i} packet-${i}`),
    ),
  };
  for (let i = 0; i < 8; i++) {
    const id = `TASK-00${30 + i}`;
    files[`orchestration/tasks/${id}-packet-${i}/SPEC.md`] = spec(
      id,
      `packet-${i}`,
    );
  }
  const dir = makeBoard(t, files);
  const { code, json } = runSentinel(dir, ["--min-ready", "9"]);
  assert.notEqual(code, 0);
  assert.equal(json.queue_floor.satisfied, false);
  assert.ok(json.errors.some((e) => e.type === "queue_floor_violation"));
});

test("coordinator remote-branch timestamps are listed when refs exist", (t) => {
  const git = spawnSync("git", ["--version"], { encoding: "utf8" });
  if (git.status !== 0) {
    t.skip("git not available");
    return;
  }
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(["TASK-0040 branchy"]),
    [`orchestration/tasks/TASK-0040-branchy/SPEC.md`]: spec(
      "TASK-0040",
      "branchy",
    ),
  });
  const run = (args, opts = {}) =>
    spawnSync("git", args, { cwd: dir, encoding: "utf8", ...opts });
  run(["init", "-b", "main"]);
  run(["config", "user.email", "sentinel@example.test"]);
  run(["config", "user.name", "sentinel"]);
  fs.writeFileSync(path.join(dir, "f.txt"), "x\n");
  run(["add", "f.txt"]);
  run(["commit", "-m", "c"]);
  const sha = run(["rev-parse", "HEAD"]).stdout.trim();
  run(["update-ref", `refs/remotes/origin/codex/${"TASK-0040"}-branchy-w1`, sha]);
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.equal(code, 0);
  assert.equal(json.counts.coordinator_branches, 1);
  assert.equal(json.coordinator_branches[0].task_id, "TASK-0040");
  assert.equal(json.coordinator_branches[0].commit, sha.slice(0, 7));
  assert.ok(json.coordinator_branches[0].committed_at);
});

test("bullet-list STATUS grammar (historical format) parses", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(["TASK-0041 bulleted"]),
    [`orchestration/tasks/TASK-0041-bulleted/SPEC.md`]: spec(
      "TASK-0041",
      "bulleted",
    ),
    [`orchestration/tasks/TASK-0041-bulleted/STATUS.md`]: `# claim

- task: TASK-0041
- state: REVIEW_REQUESTED
- coordinator: ox-pc-a
`,
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "0"]);
  assert.equal(code, 0);
  assert.equal(json.counts.review_requested, 1);
  assert.equal(json.review_requested[0].coordinator, "ox-pc-a");
});

test("contradictory READY+HOLD listing fails", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md":
      READY_TABLE(["TASK-0042 conflicted"]) +
      HOLD_SECTION(["TASK-0042 conflicted"]),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.notEqual(code, 0);
  assert.ok(json.errors.some((e) => e.type === "contradictory_state"));
});

test("annotated historical SPEC state parses by its leading state token", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(["TASK-0043 annotated"]),
    [`orchestration/tasks/TASK-0043-annotated/SPEC.md`]: `---
task: TASK-0043
state: READY (PIPELINED — claimable only AFTER TASK-0042 is INTEGRATED)
---
`,
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.equal(code, 0);
  assert.deepEqual(json.effective_ready, ["TASK-0043"]);
  assert.ok(json.spec_state_annotations["TASK-0043"].includes("PIPELINED"));
  assert.equal(json.errors.length, 0);
});

test("live CLAIMED task absent from READY table is surfaced globally", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(["TASK-0060 on-board"]),
    [`orchestration/tasks/TASK-0060-on-board/SPEC.md`]: spec(
      "TASK-0060",
      "on-board",
    ),
    [`orchestration/tasks/TASK-0061-off-board/SPEC.md`]: spec(
      "TASK-0061",
      "off-board",
    ),
    [`orchestration/tasks/TASK-0061-off-board/STATUS.md`]:
      statusFile("CLAIMED", "kimi-work (worker: kw)"),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.equal(code, 0);
  assert.deepEqual(json.effective_ready, ["TASK-0060"]);
  assert.equal(json.counts.effective_ready, 1);
  assert.equal(json.counts.claimed, 1);
  assert.equal(json.claimed[0].id, "TASK-0061");
  assert.match(json.claimed[0].coordinator, /kimi-work/);
});

test("live REVIEW_REQUESTED task absent from READY table is surfaced globally", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(["TASK-0062 on-board"]),
    [`orchestration/tasks/TASK-0062-on-board/SPEC.md`]: spec(
      "TASK-0062",
      "on-board",
    ),
    [`orchestration/tasks/TASK-0063-off-board/SPEC.md`]: spec(
      "TASK-0063",
      "off-board",
    ),
    [`orchestration/tasks/TASK-0063-off-board/STATUS.md`]:
      statusFile("REVIEW_REQUESTED", "deepseek (worker: ds)"),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.equal(code, 0);
  assert.deepEqual(json.effective_ready, ["TASK-0062"]);
  assert.equal(json.counts.review_requested, 1);
  assert.equal(json.review_requested[0].id, "TASK-0063");
});

test("global live claim colliding with a READY task fails", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(["TASK-0064 ready-worker"]),
    [`orchestration/tasks/TASK-0064-ready-worker/SPEC.md`]: `---
task: TASK-0064
state: READY
owned_paths: [shared/module/**]
---
`,
    [`orchestration/tasks/TASK-0065-hidden-claim/SPEC.md`]: `---
task: TASK-0065
state: READY
owned_paths: [shared/module/deep.cpp]
---
`,
    [`orchestration/tasks/TASK-0065-hidden-claim/STATUS.md`]:
      statusFile("IMPLEMENTED", "codex (worker: ox-pc-x)"),
  });
  const { code, json } = runSentinel(dir, ["--min-ready", "1"]);
  assert.notEqual(code, 0);
  assert.equal(json.counts.collisions, 1);
  assert.equal(
    json.collisions[0].a === "TASK-0064" || json.collisions[0].b === "TASK-0064",
    true,
  );
  assert.equal(
    json.collisions[0].a === "TASK-0065" || json.collisions[0].b === "TASK-0065",
    true,
  );
  assert.ok(json.errors.some((e) => e.type === "owned_path_collision"));
});

test("non-JSON human output keeps the same exit code", (t) => {
  const dir = makeBoard(t, {
    "orchestration/RUN_STATUS.md": READY_TABLE(
      Array.from({ length: 8 }, (_, i) => `TASK-00${50 + i} packet-${i}`),
    ),
  });
  for (let i = 0; i < 8; i++) {
    const id = `TASK-00${50 + i}`;
    dir && fs.mkdirSync(path.join(dir, "orchestration/tasks", `${id}-packet-${i}`), { recursive: true });
    fs.writeFileSync(
      path.join(dir, "orchestration/tasks", `${id}-packet-${i}`, "SPEC.md"),
      spec(id, `packet-${i}`),
    );
  }
  const res = spawnSync(
    process.execPath,
    [SENTINEL, "--repo", dir, "--min-ready", "8"],
    { encoding: "utf8" },
  );
  assert.equal(res.status, 0);
  assert.match(res.stdout, /healthy/);
});
