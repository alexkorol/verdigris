from pathlib import Path
import hashlib
import json

BASE = Path(__file__).resolve().parent
ROOT = BASE.parents[4]
TOOLS = ROOT / 'native/tools/raster'
RASTER = ROOT / 'native/client/assets/raster'

def digest(raw):
    return hashlib.sha256(raw).hexdigest()

def record(path, role):
    raw = path.read_bytes()
    binary = path.suffix.lower() in ('.png', '.gif')
    result = {'path': path.relative_to(ROOT).as_posix(), 'role': role,
              'sha256': digest(raw if binary else raw.replace(b'\r\n', b'\n')),
              'hash_policy': 'raw_binary' if binary else 'canonical_lf_crlf_to_lf_only'}
    if not binary:
        result['historical_working_file_raw_sha256'] = digest(raw)
    return result

for version in (1, 2):
    provenance = json.loads((RASTER / f'prompts/raider-strike-sw-v{version}.provenance.json').read_text())
    assert digest((ROOT / provenance['source']).read_bytes()) == provenance['sourceSha256']
    assert digest((ROOT / provenance['prompt']).read_bytes()) == provenance['promptSha256']
    assert (ROOT / provenance['prompt']).read_text().rstrip('\n') == provenance['submittedPrompt']
    for reference in provenance['references']:
        assert digest((ROOT / reference['path']).read_bytes()) == reference['sha256']
    for key in ('manifest', 'report'):
        assert digest((ROOT / provenance['conversion'][key]).read_bytes()) == provenance['conversion'][key + 'Sha256']

promotion = json.loads((BASE / 'runtime-promotion.json').read_text())
for frame in promotion['frames']:
    runtime = RASTER / 'runtime' / (frame['name'] + '.png')
    candidate = BASE / (frame['name'] + '.png')
    assert runtime.read_bytes() == candidate.read_bytes()
    assert digest(runtime.read_bytes()) == frame['sha256']
assert digest((RASTER / 'runtime/raider_sw.png').read_bytes()) == '13e515e678d001de0e764729048986d1d459c9df2de9c2b52d2d1c8538b3daf4'

deps = {
    'version': 1,
    'scope': 'Raider SW six-phase strike, root native review accepted; production event timing remains root-owned. Paths are repository relative.',
    'reproduction_boundary': 'Active import reconstructs selected v1 phases0,2,3,4,5 and v2 anticipation1 from the two original PNGs using actual Pixel Respecter, one shared32-color palette, cell6 and common source registration. Exact neutral-only alpha windows are embedded in the active manifest; no rejected candidate recipe or PNG is required.',
    'hash_policy': 'Text hashes normalize CRLF to LF only; binary hashes use raw bytes. Historical generation and candidate-report hash fields remain unchanged.',
    'active_inputs': [record(RASTER / f'source/raider-strike-sw-v{v}.png', 'raw generated source; selected phases0,2,3,4,5' if v == 1 else 'raw color repair; selected anticipation1 only') for v in (1, 2)],
    'active_recipes': [record(TOOLS / 'raider-strike-sw.json', 'active reproducible six-frame import'), record(TOOLS / 'import_assets.py', 'actual Pixel Respecter API adapter')],
    'historical_provenance': [record(RASTER / 'runtime/raider_sw.png', 'supplied original identity and axe reference, unchanged')] + [record(RASTER / f'prompts/raider-strike-sw-v{v}{suffix}', 'exact submitted prompt' if suffix == '.txt' else 'source generation and reference-role record') for v in (1, 2) for suffix in ('.txt', '.provenance.json')] + [record(TOOLS / 'raider-strike-sw-candidate.json', 'selected candidate reconstruction recipe'), record(BASE / 'raider-strike-sw-candidate.provenance.json', 'selected candidate reconstruction evidence')],
    'review_evidence': [record(BASE / name, 'selected native/alpha acceptance evidence') for name in ('acceptance-review.json', 'alpha-audit.json', 'cleanup-inspection.json', 'cycle-review.json', 'runtime-promotion.json', 'contact-4x.png', 'idle-strike-idle-1x.png', 'source-gap-inspection.png')],
    'external_project': {'path': 'Z:/Code/Python/pixel-perfecter', 'vendored': False, 'provenance': 'native/client/assets/raster/runtime/raider-strike-sw.provenance.json'},
    'limitations': ['Sources requested alpha but returned opaque checker backgrounds; final deterministic neutral-only cleanup was inspected against pale skin.', 'Anticipation raises outward instead of the exact requested rear draw; low contact and follow-through need production timing review.', 'Candidate palette and outlines have minor variation from idle; all six runtime PNGs are byte-identical to the reviewed candidate.', 'Rejected olive anticipation and overly broad cleanup remain preserved locally; rejected candidate folders are not active dependencies.']
}
dependency_path = TOOLS / 'raider-strike-sw-dependencies.json'
dependency_path.write_text(json.dumps(deps, indent=2) + '\n')

paths = [RASTER / f'source/raider-strike-sw-v{v}.png' for v in (1, 2)]
paths += [RASTER / f'prompts/raider-strike-sw-v{v}{suffix}' for v in (1, 2) for suffix in ('.txt', '.provenance.json')]
paths += [TOOLS / name for name in ('raider-strike-sw.json', 'raider-strike-sw-candidate.json', 'raider-strike-sw-v1-candidate.json', 'raider-strike-sw-dependencies.json')]
paths += [RASTER / f'runtime/raider_strike{i}_sw.png' for i in range(6)]
paths += [RASTER / 'runtime/raider-strike-sw.provenance.json']
paths += [path for path in BASE.iterdir() if path.is_file() and path.suffix.lower() in ('.json', '.png', '.gif', '.py', '.html')]
# Preserve one compact review of the rejected cleanup; the rejected PNG batch stays local.
paths += [BASE / 'rejected-color-fill' / name for name in ('rejection.json', 'manifest.json', 'contact-4x.png')]
staging_path = TOOLS / 'raider-strike-sw-staging-paths.txt'
relative = sorted(set(path.relative_to(ROOT).as_posix() for path in paths))
for path in relative:
    assert (ROOT / path).is_file(), path
relative.append(staging_path.relative_to(ROOT).as_posix())
staging_path.write_text('\n'.join(relative) + '\n')
print(json.dumps({'validated_source_prompt_reference_records': 2, 'byte_identical_runtime_frames': 6, 'dependency_manifest': dependency_path.relative_to(ROOT).as_posix(), 'exact_staging_paths': len(relative), 'staging_list': staging_path.relative_to(ROOT).as_posix()}))
