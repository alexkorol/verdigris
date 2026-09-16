# Handoff Prompt — Delaford Sprite Overhaul

## What we're doing
Overhauling all pixel art sprites in "Delaford" — a browser-based Node.js/Vite RPG. The game uses 32×32 pixel art spritesheets. Goal: redesign all sprites in a dark bronze-age fantasy style (Diablo 2 / early Path of Exile influence — aged bronze, dark iron, worn leather, torchlit atmosphere, gritty and weathered). Target output: 64×64 pixel art.

The pipeline is:
1. **Generate** new sprites via OpenRouter API (Gemini 2.5 Flash Image model) using the original sprites as visual reference + a dark-style text prompt
2. **Clean up** with `pixel-perfecter`'s `PixelArtReconstructor` to snap to proper pixel grid
3. **Review** comparison images (original | generated | reconstructed) before scaling up

We validated the pipeline concept and wrote all the code. We're at the point of **running the first 3 samples** (weapon, armor, terrain).

---

## Key paths

| What | Windows path |
|------|-------------|
| Game folder | `Z:\Code\Games\delaford\delaford_game\` |
| Sprite samples dir | `Z:\Code\Games\delaford\sprite_samples\` |
| Generator script | `Z:\Code\Games\delaford\sprite_samples\generate_samples.py` |
| Run script (Windows) | `Z:\Code\Games\delaford\sprite_samples\run_samples.bat` |
| pixel-perfecter | `Z:\Code\Python\pixel-perfecter\` |
| Weapons spritesheet | `Z:\Code\Games\delaford\delaford_game\public\assets\images\weapons.png` |
| Armor spritesheet | `Z:\Code\Games\delaford\delaford_game\public\assets\images\armor.png` |
| Terrain spritesheet | `Z:\Code\Games\delaford\delaford_game\public\assets\images\terrain.png` |

---

## What's already done

### Scripts (ready to run)
- **`generate_samples.py`** — calls OpenRouter with each original 32×32 sprite + a detailed dark-fantasy pixel art prompt, runs pixel-perfecter reconstructor, saves comparison images. Updated with strong low-res emphasis (explicitly targets 64×64, hard pixel edges, 16-32 color palette, NES/SNES-era constraints, no anti-aliasing).
- **`run_samples.bat`** — double-click on Windows to activate Anaconda and run the script. This is what needs to be triggered.

### Original sprites extracted
These are already saved in `sprite_samples\`:
- `weapon_original_32x32.png` — Bronze Sword (row 0, col 0 of weapons.png)
- `armor_original_32x32.png` — Iron Chainmail (row 4, col 1 of armor.png)
- `terrain_original_32x32.png` — dungeon stone floor tile (row 13, col 1 of terrain.png)
- `*_original_preview.png` — 8× upscaled previews of each

### pixel-perfecter (`Z:\Code\Python\pixel-perfecter\`)
- `PixelArtReconstructor` in `pixel_perfecter/reconstructor.py` — grid detection + modal color cell snapping
- Usage: `rec = PixelArtReconstructor(image=np_array); result = rec.run(mode="global")`
- **IMPORTANT from CLAUDE.md**: "NEVER run batch API calls on uncurated data — always visually QC first"

---

## API / credentials
- **OpenRouter API key** is stored in `Z:\Code\Python\pixel-perfecter\.env` as `OPENROUTER_API_KEY=sk-or-v1-...`
- The generate_samples.py script reads it automatically from that file
- **Model**: `google/gemini-2.5-flash-image` via OpenRouter
- The sandbox/Linux container CANNOT reach external APIs (blocked). Scripts must run on the user's Windows machine.

---

## The immediate next step
**Run `run_samples.bat`** by double-clicking it in File Explorer at `Z:\Code\Games\delaford\sprite_samples\`. This will:
1. Activate the Anaconda Python environment
2. Call OpenRouter (Gemini) with each of the 3 original sprites + the dark bronze-age prompt
3. Run pixel-perfecter reconstructor on each result
4. Save `weapon_generated.png`, `weapon_reconstructed.png`, `weapon_comparison.png` (and same for armor, terrain)

After it runs, review the 3 comparison images and iterate on style/prompts before scaling up to all sprites.

---

## Constraint: cannot type in terminals
The computer-use tool grants Command Prompt at "click" tier only — no keyboard input. Use **File Explorer** (full tier) to navigate to `Z:\Code\Games\delaford\sprite_samples\` and double-click `run_samples.bat`. Or ask the user to run it — it's a one-click action.

---

## Sprite architecture (for later full batch)
All sprites are 32×32 tiles in grids. Item→sprite position mappings are in:
- `delaford_game/server/core/data/items/weapons.js` — weapons by material tier (row) and type (col)
  - Rows: Bronze=0, Iron=1, Steel=2, Jatite=3
  - Cols: Sword=0, Axe=1, Pickaxe=2, Dagger=3, Mace=4, Battleaxe=5, Halberd=6, Warhammer=7, Spear=8; Bows at 17-18
- `delaford_game/server/core/data/items/armor.js` — armor by slot (row) and material (col)
- Other sheets: objects.png, tileset.png, monsters.png, npcs.png, human.png, edible.png, general.png, jewelry.png

---

## What NOT to do
- Do NOT try to download images from ChatGPT via the browser — save dialogs open, base64 is privacy-filtered, it's a dead end. Use generate_samples.py + OpenRouter directly.
- Do NOT run batch API calls until the 3 sample comparisons are reviewed and approved.
- Do NOT use Claude in Chrome to automate the ChatGPT UI for image generation — we abandoned that approach.
