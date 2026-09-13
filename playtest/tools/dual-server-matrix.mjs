/**
 * Dual-server parity matrix runner (TASK-0082, D-116 regression sweep layer 1).
 *
 * Boots a fresh JS server and a fresh C++ server on two explicit loopback
 * ports, drives the UNCHANGED playtest runner in --attach mode against each
 * serially, and writes one comparison JSON artifact.
 *
 *   node playtest/tools/dual-server-matrix.mjs \
 *     --native-exe native/build/verdigris_server.exe \
 *     --js-port 6541 --native-port 6542 \
 *     [--scenarios quickstart,movement,zones] \
 *     --out orchestration/tasks/TASK-0082-dual-server-matrix/captures/smoke.json
 *
 * With no --scenarios every current scenario runs alphabetically. The wrapper
 * owns process startup/shutdown, gives both servers isolated save paths, and
 * kills only the child processes it spawned. Any red or asymmetric scenario
 * exits non-zero. It parses ordinary runner output; it never alters the
 * runner or its assertions.
 */

import { spawn, execFile } from 'node:child_process';
import crypto from 'node:crypto';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const projectRoot = path.resolve(here, '..', '..');
const scenariosDir = path.join(projectRoot, 'playtest', 'scenarios');
const FORBIDDEN_PORT = 6500;
const READY_TIMEOUT_MS = 30000;
const RUNNER_TIMEOUT_MS = 20 * 60 * 1000;

const out = (line) => process.stdout.write(`${line}\n`);
const err = (line) => process.stderr.write(`${line}\n`);

const usage = `Usage:
  node playtest/tools/dual-server-matrix.mjs \\
    --native-exe native/build/verdigris_server.exe \\
    --js-port <PORT> --native-port <PORT> \\
    [--scenarios name,name,...] \\
    --out <artifact.json>`;

class UsageError extends Error {}

function parseArgs(argv) {
  const options = {
    nativeExe: null,
    jsPort: null,
    nativePort: null,
    scenarios: null,
    out: null,
  };
  for (let i = 0; i < argv.length; i += 1) {
    const arg = argv[i];
    const value = () => {
      if (i + 1 >= argv.length) throw new UsageError(`${arg} requires a value`);
      i += 1;
      return argv[i];
    };
    if (arg === '--native-exe') options.nativeExe = value();
    else if (arg === '--js-port') options.jsPort = value();
    else if (arg === '--native-port') options.nativePort = value();
    else if (arg === '--scenarios') options.scenarios = value();
    else if (arg === '--out') options.out = value();
    else if (arg === '--help' || arg === '-h') {
      out(usage);
      process.exit(0);
    } else throw new UsageError(`Unknown argument: ${arg}`);
  }
  if (!options.nativeExe) throw new UsageError('--native-exe is required');
  if (!options.jsPort) throw new UsageError('--js-port is required');
  if (!options.nativePort) throw new UsageError('--native-port is required');
  if (!options.out) throw new UsageError('--out is required');
  return options;
}

function resolvePort(raw, label) {
  const port = Number(raw);
  if (!Number.isInteger(port) || port <= 0 || port > 65535) {
    throw new UsageError(`--${label} must be an integer TCP port (got ${raw})`);
  }
  if (port === FORBIDDEN_PORT) {
    throw new UsageError(`port ${FORBIDDEN_PORT} is forbidden: pick ports inside your coordinator capsule`);
  }
  return port;
}

function availableScenarios() {
  return fs.readdirSync(scenariosDir)
    .filter(file => file.endsWith('.mjs'))
    .map(file => file.replace(/\.mjs$/, ''))
    .sort();
}

function resolveScenarioNames(requested) {
  const all = availableScenarios();
  if (requested === null) return all;
  const names = requested.split(',').map(name => name.trim()).filter(Boolean);
  if (!names.length) throw new UsageError('--scenarios must name at least one scenario');
  const unknown = names.filter(name => !all.includes(name));
  if (unknown.length) {
    throw new UsageError(
      `Unknown scenario(s): ${unknown.join(', ')}. Available: ${all.join(', ')}`,
    );
  }
  return [...new Set(names)];
}

const execFilePromise = (file, args, cwd) => new Promise((resolve) => {
  execFile(file, args, { cwd }, (error, stdout) => {
    resolve(error ? null : String(stdout).trim());
  });
});

async function gitRevision() {
  const full = await execFilePromise('git', ['rev-parse', 'HEAD'], projectRoot);
  if (!full) return null;
  return { full, short: full.slice(0, 9) };
}

function sha256File(filePath) {
  return new Promise((resolve, reject) => {
    const hash = crypto.createHash('sha256');
    const stream = fs.createReadStream(filePath);
    stream.on('data', chunk => hash.update(chunk));
    stream.on('error', reject);
    stream.on('end', () => resolve(hash.digest('hex')));
  });
}

function commandLine(command) {
  return command
    .map(part => (/[\s"]/.test(part) ? `"${part.replace(/"/g, '\\"')}"` : part))
    .join(' ');
}

class ChildRegistry {
  constructor() {
    this.children = new Set();
    process.on('exit', () => this.killAll());
  }

  track(child) {
    this.children.add(child);
    child.on('exit', () => this.children.delete(child));
    return child;
  }

  kill(child) {
    if (!child || child.exitCode !== null || child.signalCode !== null) return;
    try {
      child.kill();
    } catch {
      // Already gone; nothing to clean up.
    }
  }

  killAll() {
    for (const child of this.children) this.kill(child);
  }
}

const children = new ChildRegistry();

const delay = ms => new Promise(resolve => { setTimeout(resolve, ms); });

function startServer({ command, cwd, env, label, port }) {
  const child = spawn(command[0], command.slice(1), {
    cwd,
    env,
    stdio: ['ignore', 'pipe', 'pipe'],
  });
  children.track(child);

  return new Promise((resolve, reject) => {
    let output = '';
    let settled = false;
    let timer = null;
    const onData = (chunk) => {
      output += chunk.toString();
      if (/listening|started|ready/i.test(output) || output.includes(String(port))) {
        settle(null, { child, output });
      }
    };
    const onExit = (code) => {
      settle(new Error(`${label} server exited early (code ${code}).\n${output.slice(-2000)}`));
    };
    const cleanup = () => {
      clearTimeout(timer);
      child.stdout.off('data', onData);
      child.stderr.off('data', onData);
      child.off('exit', onExit);
    };
    const settle = (failure, value) => {
      if (settled) return;
      settled = true;
      cleanup();
      if (failure) reject(failure);
      else resolve(value);
    };
    timer = setTimeout(() => {
      children.kill(child);
      settle(new Error(`${label} server did not become ready within ${READY_TIMEOUT_MS}ms.\n${output.slice(-2000)}`));
    }, READY_TIMEOUT_MS);
    child.stdout.on('data', onData);
    child.stderr.on('data', onData);
    child.on('exit', onExit);
  });
}

async function stopServer(child) {
  children.kill(child);
  await Promise.race([
    new Promise((resolve) => {
      if (child.exitCode !== null || child.signalCode !== null) resolve();
      else child.once('exit', () => resolve());
    }),
    delay(5000),
  ]);
}

function runScenarios({ url, names }) {
  const command = [process.execPath, 'playtest/run.mjs', '--attach', ...names];
  const child = spawn(command[0], command.slice(1), {
    cwd: projectRoot,
    env: {
      ...process.env,
      PLAYTEST_WS_URL: url,
    },
    stdio: ['ignore', 'pipe', 'pipe'],
  });
  children.track(child);

  return new Promise((resolve) => {
    let stdout = '';
    let stderr = '';
    child.stdout.on('data', chunk => { stdout += chunk.toString(); });
    child.stderr.on('data', chunk => { stderr += chunk.toString(); });

    const timer = setTimeout(() => {
      err(`Runner against ${url} exceeded ${RUNNER_TIMEOUT_MS}ms; terminating it.`);
      children.kill(child);
    }, RUNNER_TIMEOUT_MS);

    child.on('exit', (code, signal) => {
      clearTimeout(timer);
      resolve({
        command,
        commandLine: commandLine(command),
        url,
        runnerExitCode: code,
        runnerSignal: signal,
        stdoutTail: stdout.slice(-4000),
        stderrTail: stderr.slice(-2000),
        results: parseRunnerResults(stdout),
      });
    });
  });
}

function parseRunnerResults(stdout) {
  const lineRe = /^[^\S\n]*(PASS|FAIL)[^\S\n]+(\S+)[^\S\n]+\((\d+)ms\)[^\S\n]*$/gm;
  const byName = new Map();
  let match;
  while ((match = lineRe.exec(stdout)) !== null) {
    byName.set(match[2], { ok: match[1] === 'PASS', ms: Number(match[3]) });
  }
  return byName;
}

function summarizeRun(run, names) {
  const scenarios = names.map((name) => {
    const parsed = run.results.get(name);
    return {
      name,
      ok: Boolean(parsed && parsed.ok),
      durationMs: parsed ? parsed.ms : null,
      missingFromOutput: !parsed,
    };
  });
  const failed = scenarios.filter(scenario => !scenario.ok).map(scenario => scenario.name);
  return {
    scenarios,
    passed: scenarios.length - failed.length,
    failed,
    runnerExitCode: run.runnerExitCode,
    runnerSignal: run.runnerSignal,
    complete: failed.length === 0 && scenarios.every(scenario => !scenario.missingFromOutput),
  };
}

async function main() {
  let jsPort;
  let nativePort;
  let names;
  let outPath;
  let nativeExeArg;
  let nativeExeAbs;
  try {
    const options = parseArgs(process.argv.slice(2));
    jsPort = resolvePort(options.jsPort, 'js-port');
    nativePort = resolvePort(options.nativePort, 'native-port');
    if (jsPort === nativePort) throw new UsageError('--js-port and --native-port must differ');
    names = resolveScenarioNames(options.scenarios);
    outPath = path.resolve(projectRoot, options.out);
    nativeExeArg = options.nativeExe;
    nativeExeAbs = path.resolve(projectRoot, options.nativeExe);
  } catch (error) {
    err(`${usage}\n\nERROR: ${error.message}`);
    process.exitCode = 2;
    return;
  }

  if (!fs.existsSync(nativeExeAbs)) {
    err(`ERROR: native executable not found: ${nativeExeArg} (resolved ${nativeExeAbs}).`);
    err('Build it first: powershell -File native/build.ps1');
    process.exitCode = 2;
    return;
  }

  const startedAtIso = new Date().toISOString();
  const stamp = `${process.pid}-${Date.now()}`;
  const saveRoot = path.join(os.tmpdir(), `verdigris-matrix-${stamp}`);
  fs.mkdirSync(path.join(saveRoot, 'guest'), { recursive: true });

  const revision = await gitRevision();
  const nativeExeHash = await sha256File(nativeExeAbs);
  const jsUrl = `ws://127.0.0.1:${jsPort}`;
  const nativeUrl = `ws://127.0.0.1:${nativePort}`;
  const jsCommand = [process.execPath, 'server/index.js'];
  const nativeCommand = [nativeExeAbs, String(nativePort)];
  const runnerCommandLine = commandLine([
    process.execPath, 'playtest/run.mjs', '--attach', ...names,
  ]);

  out(`Dual-server parity matrix (TASK-0082) — revision ${revision ? revision.short : 'unknown'}`);
  out(`Scenarios (${names.length}): ${names.join(', ')}`);

  let jsChild = null;
  let nativeChild = null;
  let jsServerRecord = null;
  let nativeServerRecord = null;
  let jsRun = null;
  let nativeRun = null;

  try {
    out('\n[1/4] Booting fresh JS server…');
    const jsEnv = {
      ...process.env,
      NODE_ENV: 'development',
      PORT: String(jsPort),
      PLAYER_SAVE_COOLDOWN_MS: '999999999',
      GUEST_SAVE_DIR: path.join(saveRoot, 'guest'),
      CHRONICLES_STORE_FILE: path.join(saveRoot, 'chronicles-store.json'),
      CHRONICLES_DB_FILE: path.join(saveRoot, 'chronicles.sqlite'),
    };
    const jsBootStart = Date.now();
    const jsStarted = await startServer({
      command: jsCommand,
      cwd: projectRoot,
      env: jsEnv,
      label: 'JS',
      port: jsPort,
    });
    jsChild = jsStarted.child;
    jsServerRecord = {
      kind: 'js',
      url: jsUrl,
      port: jsPort,
      command: jsCommand,
      commandLine: commandLine(jsCommand),
      pid: jsChild.pid,
      bootMs: Date.now() - jsBootStart,
      savePaths: {
        guestSaveDir: jsEnv.GUEST_SAVE_DIR,
        chroniclesStoreFile: jsEnv.CHRONICLES_STORE_FILE,
        chroniclesDbFile: jsEnv.CHRONICLES_DB_FILE,
      },
    };
    out(`JS server ready (pid ${jsChild.pid}, boot ${Date.now() - jsBootStart}ms); saves isolated under ${saveRoot}`);

    out('[2/4] Running scenarios against the JS server…');
    jsRun = await runScenarios({ url: jsUrl, names });
    await stopServer(jsChild);
    out(`JS runner finished (exit code ${jsRun.runnerExitCode}).`);

    out('\n[3/4] Booting fresh native server…');
    const nativeBootStart = Date.now();
    const nativeStarted = await startServer({
      command: nativeCommand,
      cwd: projectRoot,
      env: { ...process.env },
      label: 'Native',
      port: nativePort,
    });
    nativeChild = nativeStarted.child;
    nativeServerRecord = {
      kind: 'native',
      url: nativeUrl,
      port: nativePort,
      command: nativeCommand,
      commandLine: commandLine(nativeCommand),
      pid: nativeChild.pid,
      bootMs: Date.now() - nativeBootStart,
      exe: {
        path: nativeExeArg,
        absolutePath: nativeExeAbs,
        sha256: nativeExeHash,
      },
    };
    out(`Native server ready (pid ${nativeChild.pid}, boot ${Date.now() - nativeBootStart}ms).`);

    out('[4/4] Running scenarios against the native server…');
    nativeRun = await runScenarios({ url: nativeUrl, names });
    await stopServer(nativeChild);
    out(`Native runner finished (exit code ${nativeRun.runnerExitCode}).`);
  } finally {
    if (jsChild) await stopServer(jsChild);
    if (nativeChild) await stopServer(nativeChild);
  }

  const jsSummary = summarizeRun(jsRun, names);
  const nativeSummary = summarizeRun(nativeRun, names);
  const asymmetric = names.filter((name) => {
    const jsResult = jsRun.results.get(name);
    const nativeResult = nativeRun.results.get(name);
    return Boolean(jsResult && jsResult.ok) !== Boolean(nativeResult && nativeResult.ok);
  });
  const parity = jsSummary.complete
    && nativeSummary.complete
    && asymmetric.length === 0
    && jsRun.runnerExitCode === 0
    && nativeRun.runnerExitCode === 0;

  const artifact = {
    schemaVersion: 1,
    task: 'TASK-0082-dual-server-matrix',
    generatedAt: startedAtIso,
    completedAt: new Date().toISOString(),
    revision,
    environment: {
      node: process.version,
      platform: `${os.platform()} ${os.arch()} ${os.release()}`,
    },
    scenarios: names,
    servers: {
      js: jsServerRecord,
      native: nativeServerRecord,
    },
    saveRoot,
    runners: {
      js: {
        url: jsUrl,
        commandLine: runnerCommandLine,
        runnerExitCode: jsRun.runnerExitCode,
        runnerSignal: jsRun.runnerSignal,
      },
      native: {
        url: nativeUrl,
        commandLine: runnerCommandLine,
        runnerExitCode: nativeRun.runnerExitCode,
        runnerSignal: nativeRun.runnerSignal,
      },
    },
    runs: {
      js: jsSummary,
      native: nativeSummary,
    },
    asymmetric,
    parity,
  };

  fs.mkdirSync(path.dirname(outPath), { recursive: true });
  const tmpOut = `${outPath}.tmp-${process.pid}`;
  fs.writeFileSync(tmpOut, `${JSON.stringify(artifact, null, 2)}\n`);
  fs.renameSync(tmpOut, outPath);

  out('\n────────────────────────────────');
  for (const entry of [['js   ', jsSummary], ['native', nativeSummary]]) {
    const [label, summary] = entry;
    for (const scenario of summary.scenarios) {
      const duration = scenario.durationMs === null ? 'n/a' : `${scenario.durationMs}ms`;
      out(` ${label}  ${scenario.ok ? 'PASS' : 'FAIL'}  ${scenario.name} (${duration})`);
    }
  }
  if (asymmetric.length) out(`Asymmetric: ${asymmetric.join(', ')}`);
  out(`Artifact: ${path.relative(projectRoot, outPath)}`);
  out(parity ? 'PARITY: PASS' : 'PARITY: FAIL');
  process.exitCode = parity ? 0 : 1;
}

main().catch((error) => {
  err(`${error.stack || error}`);
  process.exitCode = 1;
});
