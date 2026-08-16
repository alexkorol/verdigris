import {cp, mkdtemp, readFile, rm} from 'node:fs/promises';
import {createServer} from 'node:http';
import {tmpdir} from 'node:os';
import {dirname, extname, isAbsolute, join, normalize, relative, resolve} from 'node:path';
import {fileURLToPath} from 'node:url';
import {spawn} from 'node:child_process';
import {runSliceChecks} from './tests/slice-checks.mjs';

const root = dirname(fileURLToPath(import.meta.url));
const committedIndex = join(root, 'index.html');

function run(command, args, options = {}) {
  return new Promise((resolvePromise, reject) => {
    const child = spawn(command, args, {cwd: options.cwd, stdio: ['ignore', 'pipe', 'pipe']});
    let stdout = '';
    let stderr = '';
    child.stdout.on('data', chunk => { stdout += chunk; });
    child.stderr.on('data', chunk => { stderr += chunk; });
    const timer = setTimeout(() => {
      child.kill();
      reject(new Error(`command timed out: ${command} ${args.join(' ')}`));
    }, options.timeout ?? 60_000);
    child.on('error', reject);
    child.on('close', code => {
      clearTimeout(timer);
      if (code !== 0) reject(new Error(`${command} exited ${code}\n${stdout}${stderr}`));
      else resolvePromise(stdout + stderr);
    });
  });
}

async function assertFreshBuild() {
  const scratch = await mkdtemp(join(tmpdir(), 'verdigris-founding-slice-'));
  try {
    await cp(join(root, 'slice.html'), join(scratch, 'slice.html'));
    await cp(join(root, 'build.mjs'), join(scratch, 'build.mjs'));
    await cp(join(root, 'assets'), join(scratch, 'assets'), {recursive: true});
    const before = await readFile(committedIndex);
    const output = await run(process.execPath, [join(scratch, 'build.mjs')], {cwd: scratch});
    const rebuilt = await readFile(join(scratch, 'index.html'));
    if (!before.equals(rebuilt)) {
      throw new Error('index.html drift detected: build output differs from the committed artifact');
    }
    const after = await readFile(committedIndex);
    if (!before.equals(after)) throw new Error('drift guard changed the committed index.html');
    return output.trim();
  } finally {
    await rm(scratch, {recursive: true, force: true});
  }
}

function mimeType(pathname) {
  return {
    '.html': 'text/html; charset=utf-8',
    '.js': 'text/javascript; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.jpeg': 'image/jpeg',
    '.svg': 'image/svg+xml',
  }[extname(pathname).toLowerCase()] || 'application/octet-stream';
}

function startServer() {
  return new Promise((resolvePromise, reject) => {
    const server = createServer(async (request, response) => {
      try {
        const requestPath = decodeURIComponent((request.url || '/').split('?')[0]);
        const requestRelative = requestPath === '/' ? 'index.html' : requestPath.replace(/^\/+/, '');
        const file = resolve(root, normalize(requestRelative));
        const containment = relative(resolve(root), file);
        if (isAbsolute(containment) || /^(?:\.\.[\\/]|\.\.$)/.test(containment)) {
          response.writeHead(403).end('forbidden');
          return;
        }
        const body = await readFile(file);
        response.writeHead(200, {'content-type': mimeType(file), 'cache-control': 'no-store'}).end(body);
      } catch (error) {
        response.writeHead(error.code === 'ENOENT' ? 404 : 500).end(error.code === 'ENOENT' ? 'not found' : 'server error');
      }
    });
    server.once('error', reject);
    server.listen(0, '127.0.0.1', () => {
      const address = server.address();
      resolvePromise({server, url: `http://127.0.0.1:${address.port}/`});
    });
  });
}

async function main() {
  const started = Date.now();
  const results = [];
  let server;
  let serverUrl;
  try {
    const buildOutput = await assertFreshBuild();
    results.push({name: 'build freshness / drift guard', ok: true, detail: buildOutput});
    ({server, url: serverUrl} = await startServer());
    const checks = await runSliceChecks(serverUrl);
    results.push(...checks);
  } catch (error) {
    results.push({name: 'harness setup', ok: false, detail: error.stack || error.message});
  } finally {
    if (server) await new Promise(resolvePromise => server.close(resolvePromise));
  }
  const failed = results.filter(result => !result.ok);
  console.log('\nFounding slice verification');
  for (const result of results) {
    console.log(`${result.ok ? 'PASS' : 'FAIL'}  ${result.name}${result.detail ? ` — ${result.detail}` : ''}`);
  }
  console.log(`\n${results.length - failed.length}/${results.length} checks passed in ${((Date.now() - started) / 1000).toFixed(1)}s`);
  if (failed.length) process.exitCode = 1;
}

await main();
