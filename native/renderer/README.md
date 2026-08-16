# Renderer seam

The first client draws placeholder shapes directly in its platform shell. A
future renderer will consume simulation snapshots/events and own billboard
projection, depth sorting, contact shadows, and procedural effects. It must not
mutate House, Scion, item, or combat state.

The WIZARD orb plates/shaders and Verdigris Splash atmosphere are planned
presentation inputs for this seam. They are not a reason to place WebGL or DOM
dependencies in the core library.

The external Claude demo plate inventory is recorded in
[`docs/rebuild/CLAUDE_DEMO_ASSET_INTAKE.md`](../../docs/rebuild/CLAUDE_DEMO_ASSET_INTAKE.md).
It maps the demo's tree, ruin, dwelling, warden, raider, and three Scion roles
to semantic billboard slots. Chroma-keying and de-spill remain renderer-side;
the native simulation sees only those semantic roles.
