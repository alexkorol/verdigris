# Prologue NPCs

The defender and well keeper (`scribe` binding) use the same anatomical Blender,
ImageGen appearance, Pixel Respecter, and depth-tested projection pipeline as
the player and village attackers. Eight static cardinal views replace the older
heavily outlined NPC sheets. Only the front idle view is currently used in play;
the other three are coherent authored views, not mirrored approximations.

The defender has a stockier build, short grey hair and jaw beard, and coarse
ochre-grey cloth. The well keeper retains long brown braided hair and a natural
grey-brown tunic. Both have empty hands. Their actual geometry supplies alpha,
silhouette,48px/metre scale and96x96canvas with anchor[48,80].

Inputs, exact built-in imagegen prompts, original RGBA outputs, native painted
atlases, projection weights, source hashes and native contact sheets are retained.
Use `render_village_attackers.py` with defender/scribe identity and the respective
male/female source scenes; transfer with `transfer_village_attackers.py`, package
with `package_village_attackers.py --npcs`. Packed editable scenes are retained
locally and included in the companion source archive. No extra NPC animations
were generated for this slice.
