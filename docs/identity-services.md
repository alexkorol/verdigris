# Identity and Role-Play Name Validation

Delaford validates character names server-side before they can be bound to an account. This document outlines how the
validation pipeline works, what operational considerations apply, and where moderation fits in.

## Pipeline overview

1. **Client request** – The character creation UI posts the desired name and the player's account identifier to
   `POST /api/identity/name-validations`. Names are capped at 64 characters and account ids at 64 word/hyphen characters;
   anything larger is rejected with a 400 before a job is created.
2. **Job creation** – `NameValidationService.createJob` normalises the name and checks the decision cache. Cached decisions
   return immediately as a `complete` job (HTTP 200); otherwise the job is enqueued as `pending` (HTTP 202) and processed by
   the configured provider.
3. **Provider decision** – The name is evaluated by the configured provider (external HTTP LLM endpoint or the built-in
   heuristic fallback). Results are cached under the lowercased normalised name.
4. **Identity binding** – When the request carries an `accountId`, the decision is appended to the account's validation
   history in the identity registry (`identity_accounts` table). A successful decision also becomes the account's
   `boundIdentity`; a failed one leaves any previous binding untouched.
5. **Polling** – The client polls `GET /api/identity/name-validations/:jobId` until the job completes. Unknown job ids return
   404. Responses carry `{ jobId, status, requestedAt, completedAt?, result? }`. The bound identity and full history are
   available via `GET /api/identity/accounts/:accountId` (404 for unknown accounts).

## Provider configuration

The validation provider is selected with environment variables:

- `NAME_VALIDATION_PROVIDER` – `http` to forward to an external LLM endpoint, defaults to `local` heuristic validation.
- `NAME_VALIDATION_ENDPOINT` – URL for the upstream service when using the `http` provider.
- `NAME_VALIDATION_API_KEY` – Optional Bearer token supplied with upstream requests.
- `NAME_VALIDATION_CACHE_TTL` – Cache lifetime in milliseconds (default 15 minutes).
- `NAME_VALIDATION_MIN_LENGTH` / `NAME_VALIDATION_MAX_LENGTH` – Bounds enforced before a provider call (defaults 3 / 24).
- `NAME_VALIDATION_JOB_TTL_MS` / `NAME_VALIDATION_MAX_JOBS` – In-memory job retention (defaults 1 hour / 1000 jobs). Jobs are
  pruned on every insert so the endpoint cannot be used as a memory-exhaustion vector.

## Rate limits and batching

- All `/api` routes sit behind a per-IP HTTP rate limit; name validation is no exception. Expect 429 responses under bursty
  load and back off client-side.
- Requests are serialized through an in-memory queue: only one name is processed at a time. Cached results are returned
  instantly, so repeat registrations never re-hit the provider.
- The HTTP endpoint accepts payloads up to 32 KB (`express.json({ limit: '32kb' })`). Keep client submissions small to avoid
  rejection.

## Persistence

Account and identity data lives in the SQLite identity database (`server/data/verdigris.sqlite` by default, overridable with
`IDENTITY_DB_FILE`). Validation history and the bound identity are stored per account in the `identity_accounts` table; login
credentials live separately in `login_accounts`. See `docs/deployment.md` ("Persistence and backups") for file locations,
backup guidance, and related environment variables — those details are not duplicated here.

## Moderation overrides

- All validation attempts (successes and failures) are recorded in the account's `history` in `identity_accounts`; the latest
  successful decision is the `boundIdentity`.
- To override a decision, update the account's `state_json` row directly (adjust `boundIdentity`, annotate or remove the
  history entry) while the server is stopped, then restart. Back up the SQLite file first — see `docs/deployment.md`.
- When forcing a new identity, prefer creating a fresh validation job with the desired name so the history captures the
  override and the cache is refreshed.
