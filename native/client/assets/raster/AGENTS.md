# Image generation for world pixel art

These rules apply to player, monster, scenery, terrain, and held-item art.
Read `PROMPTING.md` and `RESEARCH.md` before generating or accepting assets.

- Keep **one character and one direction per motion study**. Reference-led
  single-character motion sheets have real 2.5 examples; do not ban them.
  Use individual reference-led frames when sheet poses, anatomy or layout fail.
- Use a visually accepted, isolated reference image to preserve identity.
  Inspect local references with `view_image` before passing their paths.
- Keep the identity anchor in subsequent animation requests; change only
  the requested pose or facing. Do not compound drift by referencing only
  the immediately preceding frame.
- Assemble production atlases and fix integer pivots in the asset tools.
  A requested tile grid is not a verified output grid. Avoid mixing multiple
  characters and multiple directions in the same generation.
- World art must read as pixel art at its final gameplay scale. Source
  resolution, a pixel-art prompt, or a PNG extension proves nothing.
- Process through the owner's Pixel Respecter project, retain provenance,
  and inspect the reconstructed assets before enabling them in the game.
- Use natural material colors. The project name is not an art direction.
  Do not put the game name, pervasive patina, or oxidized-green styling in
  prompts. Local vegetation and deliberately weathered objects may be green.
- Request real alpha transparency. Verify the channel and enclosed gaps;
  a drawn transparency pattern is a failed output.
- Check facing, anatomical handedness, frame progression, scale, silhouette,
  and ground contact visually. Metadata and requested filenames are not proof.
- Preserve rejected source candidates outside runtime and record why each
  failed. Iterate the failed property explicitly, using accepted references.
- Never claim a specific image model was selected unless the tool exposes
  that control or identifies the model in its result.
- Acceptance requires a viewed production-game capture, animation checks,
  the existing gameplay gates, and the unchanged frame-budget limit.
