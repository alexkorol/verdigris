# VG-GPU-004 — native reference scene (planning ID)

The software backend presents the **live session** render list after it is
copied to handle-free packets. World, actors, effects, and HUD must be in
that packet stream. The isolated textured-quad sample (VG-GPU-001) is not
this scene.

Negative: a disconnected GPU demo (no session present, empty packets)
cannot pass.

Acceptance: `native/build/verdigris_client.exe --scenario gpu-reference`
