# OI-003 — production asset and font policy

**State:** WAITING_EVIDENCE on TASK-0093 and TASK-0094. **Deadline:** before any
UNKNOWN/BLOCKED asset or font ships; procedural placeholders remain valid.

Decision required: approve one provenance policy for production fonts, image
plates, generated assets, derivative maps, and package inclusion.

Recommended choice: ship only repository-tracked assets with recorded source,
license/rights, SHA-256, dimensions, derivative instructions, and named package
consumer; keep all unknown external plates reference-only.

Viable alternatives: owner-created originals under the same manifest; or a
licensed third-party pack/font with archived license and attribution terms.
Reject undocumented copying or implicit AI-generation rights.

If the owner later generates an asset, the task packet must supply the exact
prompt/instructions, pixel dimensions, color-space/alpha needs, PNG (lossless)
source filename, target folder, acceptance rubric, and any collision/normal/
mask/atlas derivatives before generation begins. None are requested now.
Fallback: procedural/vector presentation, audit work, and backend-neutral text
metrics continue.
