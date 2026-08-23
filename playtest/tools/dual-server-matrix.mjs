/**
 * Dual-server parity matrix runner (TASK-0082).
 *
 * Boots a fresh JS server and a fresh native C++ server on explicit loopback
 * ports, runs the UNCHANGED playtest scenarios serially against each via
 * `playtest/run.mjs --attach`, writes one comparison JSON artifact, and exits
 * non-zero on any red or asymmetric scenario.
 *
 *   node playtest/tools/dual-server-matrix.mjs \
 *     --native-exe native/build/verdigris_server.exe \
 *     --js-port 6541 --native-port 6542 \
 *     --scenarios quickstart,movement,zones \
 *     --out orchestration/tasks/TASK-0082-dual-server-matrix/captures/smoke.json
 *
 * With no --scenarios every current scenario runs alphabetically. This tool
 * owns startup/shutdown of ONLY the processes it spawns and never touches
 * port 6500.
 */

import { spawn, execFile as execFileCb } from 'node:child_process';
import crypto from 'node:crypto';
import fs from 'node:fs';
import http from 'node:http';
import net from 'node:net';
import os from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { promisify } from 'node:util';

const execFile = promisify(execFileCb);

const here = path.dirname(fileURLToPath(import.meta.url));
const projectRoot = path.resolve(here, '..', '..');
const scenariosDir = path.join(projectRoot, 'playtest', 'scenarios');

const FORBIDDEN_PORT = 6500;
const SERVER_READY_TIMEOUT_MS = 45000;
const SERVER_PROBE_INTERVAL_MS = 400;
const RUNNER_TIMEOUT_MS = 15 * 60 * 1000;
const OUTPUT_TAIL_BYTES = 12000;

const log = (line) => process.stdout.write(`${line}\n`);
const fail = (line) => process.stderr.write(`${line}\n`);

const usage = () => [
  'Usage:',
  '  node playtest/tools/dual-server-matrix.mjs \\',
  '    --native-exe native/build/verdigris_server.exe \\',
  '    --js-port <PORT> --native-port <PORT> \\',
  '    [--scenarios name1,name2,...] \\',
  '    --out <artifact.json>',
  '',
  'Ports must be two distinct explicit loopback capsule ports (never 6500).',
].join('\n');

const parseArgs = (argv) => {
  const parsed = {};
  for (let i = 0; i < argv.length; i += 1) {
    const arg = argv[i];
    if (!arg.startsWith('--')) {
      throw new Error(`unexpected positional argument: ${arg}`);
    }
    const key = arg.slice(2);
    const value = argv[i + 1];
    if (value === undefined || value.startsWith('--')) {
      throw new Error(`missing value for --${key}`);
    }
    if (parsed[key] !== undefined) {
      throw new Error(`duplicate option --${key}`);
    }
    parsed[key] = value;
    i += 1;
  }
  return parsed;
};

const parsePort = (raw, label) => {
  const value = Number(raw);
  if (!Number.isInteger(value) || value < 1024 || value > 65535) {
    throw new Error(`--${label} must be an integer TCP port between 1024 and 65535 (got "${raw}")`);
  }
  if (value === FORBIDDEN_PORT) {
    throw new Error(`port ${FORBIDDEN_PORT} is forbidden for the dual-server matrix`);
  }
  return value;
};

const discoverScenarios = () => fs.readdirSync(scenariosDir)
  .filter((file) => file.endsWith('.mjs'))
  .map((file) => file.replace(/\.mjs$/, ''))
  .sort();

const resolveExecutionOrder = (requestedRaw) => {
  const available = discoverScenarios();
  if (!requestedRaw) {
    return { available, executionOrder: available };
  }
  const requested = [...new Set(requestedRaw.split(',').map((name) => name.trim()).filter(Boolean))];
  if (!requested.length) {
    throw new Error('--scenarios parsed to an empty list');
  }
  const unknown = requested.filter((name) => !available.includes(name));
  if (unknown.length) {
    throw new Error(`unknown scenario(s): ${unknown.join(', ')}. Available: ${available.join(', ')}`);
  }
  // The unchanged runner executes its alphabetical master list filtered to the
  // request, so the effective order is alphabetical regardless of input order.
  return { available, executionOrder: available.filter((name) => requested.includes(name)) };
};

const revision = async () => {
  try {
    const { stdout } = await execFile('git', ['rev-parse', 'HEAD'], { cwd: projectRoot });
    return stdout.trim();
  } catch {
    return null;
  }
};

const sha256File = (file) => new Promise((resolve, reject) => {
  const hash = crypto.createHash('sha256');
  fs.createReadStream(file)
    .on('data', (chunk) => hash.update(chunk))
    .on('end', () => resolve(hash.digest('hex')))
    .on('error', reject);
});

const isPortFree = (port) => new Promise((resolve) => {
  const prober = net.createServer();
  prober.once('error', () => resolve(false));
  prober.listen(port, '127.0.0.1', () => prober.close(() => resolve(true)));
});

// Minimal WebSocket handshake probe using core modules only: a successful
// HTTP 101 upgrade proves a playable WS endpoint is listening.
const probeWebSocket = (port) => new Promise((resolve) => {
  const request = http.get({
    host: '127.0.0.1',
    port,
    path: '/',
    headers: {
      Connection: 'Upgrade',
      Upgrade: 'websocket',
      'Sec-WebSocket-Key': crypto.randomBytes(16).toString('base64'),
      'Sec-WebSocket-Version': '13',
    },
    timeout: 2000,
  });
  let settled = false;
  const finish = (ok) => {
    if (settled) return;
    settled = true;
    request.destroy();
    resolve(ok);
  };
  request.on('upgrade', () => finish(true));
  request.on('response', (response) => {
    response.resume();
    finish(false);
  });
  request.on('timeout', () => finish(false));
  request.on('error', () => finish(false));
});

const waitForServer = async (label, port) => {
  const deadline = Date.now() + SERVER_READY_TIMEOUT_MS;
  while (Date.now() < deadline) {
    if (await probeWebSocket(port)) {
      log(`[${label}] ready at ws://127.0.0.1:${port}`);
      return true;
    }
    await new Promise((resolve) => { setTimeout(resolve, SERVER_PROBE_INTERVAL_MS); });
  }
  return false;
};

const children = new Set();

const spawnTracked = (file, args, options) => {
  const child = spawn(file, args, options);
  children.add(child);
  child.once('exit', () => children.delete(child));
  return child;
};

const killSpawnedOnly = () => {
  for (const child of children) {
    try {
      if (child.exitCode === null && !child.killed) child.kill();
    } catch {
      // best effort; never mask the primary failure
    }
  }
};

const tail = (buffer) => (buffer.length > OUTPUT_TAIL_BYTES
  ? `…${buffer.slice(-OUTPUT_TAIL_BYTES)}`
  : buffer);

const collectLines = (stream, onLine) => {
  stream.setEncoding('utf8');
  let pending = '';
  stream.on('data', (chunk) => {
    pending += chunk;
    const parts = pending.split(/\r?\n/);
    pending = parts.pop();
    parts.forEach(onLine);
  });
  stream.on('end', () => {
    if (pending) onLine(pending);
  });
};

const startJsServer = ({ port, stamp }) => {
  const command = {
    file: process.execPath,
    args: ['server/index.js'],
    cwd: projectRoot,
  };
  const savePaths = {
    GUEST_SAVE_DIR: path.join(os.tmpdir(), `verdigris-matrix-js-${stamp}`),
    CHRONICLES_STORE_FILE: path.join(os.tmpdir(), `verdigris-matrix-chronicles-${stamp}.json`),
    CHRONICLES_DB_FILE: path.join(os.tmpdir(), `verdigris-matrix-${stamp}.sqlite`),
  };
  // Loopback-only, hermetic saves: pinned explicitly so inherited shell env
  // can never widen the listener or leak developer characters into the run.
  const envDelta = {
    NODE_ENV: 'development',
    PORT: String(port),
    PLAYER_SAVE_COOLDOWN_MS: '999999999',
    VERDIGRIS_BIND_HOST: '127.0.0.1',
    ...savePaths,
  };
  const child = spawnTracked(command.file, command.args, {
    cwd: command.cwd,
    env: { ...process.env, ...envDelta },
    stdio: ['ignore', 'pipe', 'pipe'],
  });
  return { label: 'js', port, child, command, envDelta, savePaths };
};

const startNativeServer = async ({ exePath, port }) => {
  const resolvedExe = path.resolve(projectRoot, exePath);
  if (!fs.existsSync(resolvedExe)) {
    throw new Error(`native executable not found: ${resolvedExe}`);
  }
  const command = {
    file: resolvedExe,
    args: [String(port)],
    cwd: projectRoot,
  };
  // argv[1] carries the port; VERDIGRIS_PORT is scrubbed so it cannot win.
  const { VERDIGRIS_PORT: _ignored, ...cleanEnv } = process.env;
  const envDelta = { VERDIGRIS_PORT: undefined };
  const child = spawnTracked(command.file, command.args, {
    cwd: command.cwd,
    env: cleanEnv,
    stdio: ['ignore', 'pipe', 'pipe'],
  });
  return {
    label: 'native',
    port,
    child,
    command,
    envDelta,
    executable: { path: resolvedExe, sha256: await sha256File(resolvedExe) },
  };
};

const summarizeServerOutput = (server, buffers) => {
  collectLines(server.child.stdout, (line) => {
    buffers.stdout += `${line}\n`;
  });
  collectLines(server.child.stderr, (line) => {
    buffers.stderr += `${line}\n`;
  });
};

const runScenarioSuite = async ({ server, scenarioNames }) => {
  log(`[${server.label}] running ${scenarioNames.length} scenario(s): ${scenarioNames.join(', ')}`);
  const command = {
    file: process.execPath,
    args: ['playtest/run.mjs', '--attach', ...scenarioNames],
    cwd: projectRoot,
  };
  const envDelta = { PLAYTEST_WS_URL: `ws://127.0.0.1:${server.port}` };
  const startedAt = Date.now();
  const child = spawnTracked(command.file, command.args, {
    cwd: command.cwd,
    env: { ...process.env, ...envDelta },
    stdio: ['ignore', 'pipe', 'pipe'],
  });

  let stdoutText = '';
  let stderrText = '';
  let timedOut = false;
  let spawnError = null;

  const passPattern = /^\s*PASS\s+(\S+)\s+\((\d+)ms\)\s*$/;
  const failPattern = /^\s*FAIL\s+(\S+):\s*(.*)$/;
  const summaryPattern = /(\d+)\/(\d+) scenarios passed/;
  const outcomes = new Map();
  let summaryLine = null;

  const handleLine = (line) => {
    stdoutText += `${line}\n`;
    const passMatch = line.match(passPattern);
    if (passMatch) {
      outcomes.set(passMatch[1], { name: passMatch[1], ok: true, ms: Number(passMatch[2]) });
    }
    const failMatch = line.match(failPattern);
    if (failMatch) {
      outcomes.set(failMatch[1], { name: failMatch[1], ok: false, ms: null, error: failMatch[2] });
    }
    const summaryMatch = line.match(summaryPattern);
    if (summaryMatch) summaryLine = summaryMatch[0];
    log(`[${server.label}] ${line}`);
  };

  collectLines(child.stdout, handleLine);
  collectLines(child.stderr, (line) => {
    stderrText += `${line}\n`;
    log(`[${server.label}][stderr] ${line}`);
  });

  const watchdog = setTimeout(() => {
    timedOut = true;
    try {
      child.kill();
    } catch {
      // already gone
    }
  }, RUNNER_TIMEOUT_MS);

  const code = await new Promise((resolve) => {
    child.once('error', (error) => {
      spawnError = error;
      resolve(null);
    });
    child.once('exit', (exitCode) => resolve(exitCode));
  });
  clearTimeout(watchdog);
  const durationMs = Date.now() - startedAt;

  const scenarios = scenarioNames.map((name) => outcomes.get(name) || {
    name,
    ok: false,
    ms: null,
    error: timedOut ? 'matrix runner timed out' : 'scenario produced no result',
  });

  return {
    command,
    envDelta,
    exitCode: timedOut ? 1 : code,
    durationMs,
    scenarios,
    summaryLine,
    stdoutTail: tail(stdoutText),
    stderrTail: tail(stderrText),
    ...(timedOut ? { timedOut: true } : {}),
    ...(spawnError ? { spawnError: String(spawnError) } : {}),
  };
};

const buildServerRecord = async ({ server, buffers, suite }) => ({
  label: server.label,
  url: `ws://127.0.0.1:${server.port}`,
  command: server.command,
  envDelta: server.envDelta,
  executable: server.executable || {
    path: process.execPath,
    sha256: await sha256File(process.execPath),
  },
  ...(server.savePaths ? { isolatedSavePaths: server.savePaths } : {}),
  runner: {
    command: suite.command,
    envDelta: suite.envDelta,
    exitCode: suite.exitCode,
    durationMs: suite.durationMs,
    summaryLine: suite.summaryLine,
  },
  scenarios: suite.scenarios,
  serverStdoutTail: tail(buffers.stdout),
  serverStderrTail: tail(buffers.stderr),
});

const computeParity = (executionOrder, records) => {
  const [js, native] = records;
  const jsByName = new Map(js.scenarios.map((entry) => [entry.name, entry]));
  const nativeByName = new Map(native.scenarios.map((entry) => [entry.name, entry]));
  const asymmetries = [];
  for (const name of executionOrder) {
    const jsEntry = jsByName.get(name);
    const nativeEntry = nativeByName.get(name);
    if (!jsEntry || !nativeEntry) {
      asymmetries.push({ scenario: name, reason: `missing result on ${!jsEntry ? 'js' : 'native'}` });
      continue;
    }
    if (jsEntry.ok !== nativeEntry.ok) {
      asymmetries.push({
        scenario: name,
        js: jsEntry.ok,
        native: nativeEntry.ok,
        reason: 'asymmetric outcome',
      });
    }
  }
  const allGreen = executionOrder.every((name) => {
    const jsEntry = jsByName.get(name);
    const nativeEntry = nativeByName.get(name);
    return jsEntry?.ok === true && nativeEntry?.ok === true;
  });
  const cleanExits = js.runner.exitCode === 0 && native.runner.exitCode === 0;
  return {
    parity: executionOrder.length > 0 && allGreen && cleanExits && asymmetries.length === 0,
    asymmetries,
  };
};

const main = async () => {
  let options;
  try {
    options = parseArgs(process.argv.slice(2));
  } catch (error) {
    fail(`${usage()}\n\nERROR: ${error.message}`);
    process.exitCode = 2;
    return;
  }

  const { 'native-exe': nativeExe, 'js-port': jsPortRaw, 'native-port': nativePortRaw, out } = options;

  const requiredMissing = [
    ['--native-exe', nativeExe],
    ['--js-port', jsPortRaw],
    ['--native-port', nativePortRaw],
    ['--out', out],
  ].filter(([, value]) => value === undefined).map(([flag]) => flag);
  if (requiredMissing.length) {
    fail(`${usage()}\n\nERROR: missing required option(s): ${requiredMissing.join(', ')}`);
    process.exitCode = 2;
    return;
  }

  let jsPort;
  let nativePort;
  let selection;
  try {
    jsPort = parsePort(jsPortRaw, 'js-port');
    nativePort = parsePort(nativePortRaw, 'native-port');
    if (jsPort === nativePort) {
      throw new Error('--js-port and --native-port must differ');
    }
    selection = resolveExecutionOrder(options.scenarios);
  } catch (error) {
    fail(`ERROR: ${error.message}`);
    process.exitCode = 2;
    return;
  }

  if (!fs.existsSync(path.resolve(projectRoot, nativeExe))) {
    fail(`ERROR: --native-exe not found: ${path.resolve(projectRoot, nativeExe)}`);
    process.exitCode = 2;
    return;
  }

  for (const [label, port] of [['js', jsPort], ['native', nativePort]]) {
    if (!(await isPortFree(port))) {
      fail(`ERROR: ${label} port ${port} is already in use; pick a free capsule port.`);
      process.exitCode = 2;
      return;
    }
  }

  const stamp = `${Date.now()}-${process.pid}`;
  const headSha = await revision();
  log(`[dual-server-matrix] revision ${headSha ?? '(git unavailable)'}`);
  log(`[dual-server-matrix] scenarios: ${selection.executionOrder.join(', ')}`);

  let jsRecord = null;
  let nativeRecord = null;
  let fatal = null;

  try {
    const jsServer = startJsServer({ port: jsPort, stamp });
    const jsBuffers = { stdout: '', stderr: '' };
    summarizeServerOutput(jsServer, jsBuffers);
    log(`[js] starting: ${process.execPath} server/index.js (port ${jsPort})`);
    const nativeServer = await startNativeServer({ exePath: nativeExe, port: nativePort });
    const nativeBuffers = { stdout: '', stderr: '' };
    summarizeServerOutput(nativeServer, nativeBuffers);
    log(`[native] starting: ${nativeServer.command.file} ${nativePort}`);

    const spawned = jsServer.child.pid !== undefined;
    if (!spawned) {
      throw new Error('JS server process failed to spawn');
    }
    if (!(await waitForServer('js', jsPort))) {
      throw new Error(`JS server not reachable at ws://127.0.0.1:${jsPort} within ${SERVER_READY_TIMEOUT_MS}ms\n${tail(jsBuffers.stdout + jsBuffers.stderr)}`);
    }
    if (nativeServer.child.pid === undefined) {
      throw new Error('native server process failed to spawn');
    }
    if (!(await waitForServer('native', nativePort))) {
      throw new Error(`native server not reachable at ws://127.0.0.1:${nativePort} within ${SERVER_READY_TIMEOUT_MS}ms\n${tail(nativeBuffers.stdout + nativeBuffers.stderr)}`);
    }

    const jsSuite = await runScenarioSuite({ server: jsServer, scenarioNames: selection.executionOrder });
    const nativeSuite = await runScenarioSuite({ server: nativeServer, scenarioNames: selection.executionOrder });

    jsRecord = await buildServerRecord({ server: jsServer, buffers: jsBuffers, suite: jsSuite });
    nativeRecord = await buildServerRecord({ server: nativeServer, buffers: nativeBuffers, suite: nativeSuite });

    log('[dual-server-matrix] ────────────────────────');
    for (const name of selection.executionOrder) {
      const jsEntry = jsSuite.scenarios.find((entry) => entry.name === name);
      const nativeEntry = nativeSuite.scenarios.find((entry) => entry.name === name);
      log(` ${jsEntry?.ok && nativeEntry?.ok ? 'PASS' : 'FAIL'}  ${name}  js=${jsEntry?.ok ? `${jsEntry.ms}ms` : 'RED'} native=${nativeEntry?.ok ? `${nativeEntry.ms}ms` : 'RED'}`);
    }
  } catch (error) {
    fatal = String(error?.stack || error);
    fail(`[dual-server-matrix] FATAL: ${fatal}`);
  } finally {
    killSpawnedOnly();
  }

  const records = [jsRecord, nativeRecord].filter(Boolean);
  const { parity, asymmetries } = records.length === 2
    ? computeParity(selection.executionOrder, records)
    : { parity: false, asymmetries: [{ reason: 'matrix aborted before both suites completed', ...(fatal ? { error: fatal } : {}) }] };

  const artifact = {
    task: 'TASK-0082-dual-server-matrix',
    generatedAt: new Date().toISOString(),
    revision: headSha,
    requestedScenarios: options.scenarios ? options.scenarios.split(',').map((name) => name.trim()).filter(Boolean) : selection.available,
    executionOrder: selection.executionOrder,
    parity,
    asymmetries,
    servers: {},
    ...(records.length === 2 ? {} : { fatal }),
  };
  if (jsRecord) artifact.servers.js = jsRecord;
  if (nativeRecord) artifact.servers.native = nativeRecord;

  const outFile = path.resolve(projectRoot, out);
  fs.mkdirSync(path.dirname(outFile), { recursive: true });
  fs.writeFileSync(outFile, `${JSON.stringify(artifact, null, 2)}\n`);

  log(`[dual-server-matrix] parity: ${parity ? 'PASS' : 'FAIL'}${asymmetries.length ? ` (${asymmetries.length} asymmetry/asymmetries)` : ''}`);
  log(`[dual-server-matrix] artifact: ${path.relative(projectRoot, outFile)}`);
  process.exitCode = parity ? 0 : 1;
};

for (const signal of ['SIGINT', 'SIGTERM', 'SIGBREAK']) {
  process.on(signal, () => {
    killSpawnedOnly();
    process.exit(130);
  });
}
process.on('uncaughtException', (error) => {
  killSpawnedOnly();
  fail(`[dual-server-matrix] uncaught exception: ${error?.stack || error}`);
  process.exit(1);
});

main().catch(async (error) => {
  killSpawnedOnly();
  fail(`[dual-server-matrix] ${error?.stack || error}`);
  process.exit(1);
});
