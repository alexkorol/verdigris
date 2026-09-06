# VG-ART-002 — bronze/stone material family (planning ID)

Cooked maps: `native/client/assets/production/bronze_stone.hpp`
(`kAlbedo`, `kRim`, SPDX CC0-1.0, source `cooked:bronze-stone-v1`).

Ruin/gate/shrine scenery samples `gdi_stone()` / rim; the GPU software
quad samples the same tables through VG-GPU-003 bindings.

Negative: magenta `kPlaceholder` cannot pass as a finished material.

Acceptance: `native/build/verdigris_client.exe --scenario bronze-stone`
