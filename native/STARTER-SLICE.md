# Village Palisade starter slice

Normal native play now admits a newly created Scion through the Village Palisade opening. Existing admitted saves retain the Crossroads entry. The isolated local-core sandbox keeps its older test loop; use `native/tools/play-native.ps1` for the actual server-backed game.

Choose Field hand, Scout or Scribe (one permanent Strength, Dexterity or Intelligence point). Speak to the defender to take and equip the weathered branch, defeat two packs, regroup at the well, defeat the breaker, earn level 2 and the first skill point, then leave through the north passage for Crossroads. A lethal hit in the opening returns the character to safety with equipment intact. Individual kills pay no XP or loot, and completed encounters cannot duplicate the victory reward. Progress survives reconnection and disk saves, separately for each Scion.

Controls: WASD moves; aim with the mouse and hold left mouse to strike. Space dashes. The visible buttons or keys 1–3 choose occupation; F speaks/regroups/leaves when the current objective allows it. The Character panel opens progression after victory. Moving onto the passage never skips the crisis.

## Implementation and validation

`ProtocolSession` owns the progression, admission, rewards and persistence. `WorldSimulation` supplies the village collision map and actual combat actors. The client submits typed intents and draws the replicated objective. Scenery, paths, collision, interaction locations and the normal scene transition all use the same village coordinates. The scene has no old portal or generic stone collision-wall rendering.

`native/build.ps1 -RunTests` includes real-combat starter authority tests. `verdigris_client.exe --scenario starter-slice` exercises the real socket/reducer/painter journey and captures the opening and encounter. `native/tools/verify-native.ps1` runs the complete native acceptance suite.

## Art scope

`assets/first-slice/starter-minimal` selects existing Blender/ImageGen character material and rig renders into one idle, four walk, six strike, two hit and four death frames per cardinal view for the two appearances and unarmed/branch states. Faster locomotion shares the same walk set. No new character animation batch was generated. The runtime copies these pixels unchanged, with 48 pixels/metre and fixed padded anchors. Character creation previews use that same current family.

`assets/first-slice/village-v3` contains four new Blender-guided ImageGen props and measured fixed-grid transfer provenance. The first failed paint remains a rejected source, never installed. Source paint retains small silhouette deviations recorded in its review. The village uses fixed daylight and no depth-of-field blur so the pixel result is inspectable.

Enemy visuals currently reuse the existing human raider family at the same world-pixel density. Their final creature design remains open; neither the rejected wolves nor the old hut/tree/portal direction is being declared approved. This is a playable starter-slice milestone, not a claim that all final art has owner approval.
