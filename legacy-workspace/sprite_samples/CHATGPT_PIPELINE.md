# Delaford Sprite Pipeline — ChatGPT web app route

Generate sprites in the **ChatGPT web app**, capture them straight into this folder
(no save dialogs), then clean them up with pixel-perfecter. No OpenRouter / API keys.

## One-time

Nothing to install if pixel-perfecter already runs (numpy + Pillow). The capture
server uses only Python's standard library.

## Per-sprite workflow

1. **Start the capture server.** Double-click `run_capture_server.bat`.
   Leave the window open. It listens on `http://localhost:8765` and saves whatever
   the browser sends into this folder. (Equivalent: `python receive_images.py`.)

2. **Generate in ChatGPT.** In the ChatGPT tab, attach the reference sprite
   (e.g. `weapon_original_32x32.png`) and paste the prompt below. Let it render the image.
   Optional but recommended: click the image to open it full-size for max resolution.

3. **Capture it.** Open DevTools in the ChatGPT tab (F12) → Console. Paste the contents
   of `capture_chatgpt.js`. Before pasting, set `FILENAME` at the top to the sprite you're
   saving, e.g. `weapon_generated.png`, `armor_generated.png`, `terrain_generated.png`.
   You should see `[capture] ✅ Saved -> ...` in the console and `Saved: ...` in the server window.
   - If the POST is blocked, change `localhost` to `127.0.0.1` in the snippet.
   - If "Could not read image bytes (CORS)", open the image full-size first, then rerun.

4. **Clean up + compare.** Once you've captured one or more `*_generated.png`, double-click
   `run_reconstruct.bat` (processes every captured sprite), or run
   `python reconstruct_samples.py weapon` for just one. It writes:
   - `<label>_reconstructed.png` — pixel-perfecter cleaned, grid-snapped
   - `<label>_comparison.png` — Original | ChatGPT generated | cleaned, side by side

5. **Review** the `*_comparison.png` files. Iterate on the prompt before scaling to all sprites.

## Files

| File | Role |
|------|------|
| `run_capture_server.bat` | Starts the local capture server (double-click) |
| `receive_images.py` | The server: receives browser POSTs, saves PNGs here |
| `capture_chatgpt.js` | Paste in ChatGPT DevTools console to send the image |
| `run_reconstruct.bat` | Runs reconstruction on captured images (double-click) |
| `reconstruct_samples.py` | pixel-perfecter cleanup + comparison builder |

`generate_samples.py` / `run_samples.bat` are the old OpenRouter route — not used here.

## The dark bronze-age prompt

> Redesign this sprite as true low-resolution pixel art — as if hand-made in a pixel
> editor at ~64x64, not a high-res "pixel art style" render. Every pixel is one solid
> square: no gradients within a pixel, no anti-aliasing between pixels, hard blocky edges,
> visibly pixelated like an SNES RPG item icon or a Diablo 2 inventory icon. Max 16–32 colors.
>
> Style: dark, gritty, low fantasy bronze age (Diablo 2 item icons, early Path of Exile).
> Palette of aged bronze, dark iron, worn leather, ochre, rust, dark sienna, charcoal, bone —
> no bright/saturated colors. Torchlit mood: warm amber highlights, deep cool shadows, strong
> contrast. Weathered and ancient: scratches, wear marks, patina on metal. Near-black
> (#0a0a0a) or transparent background. 3–4 shading values per surface. Keep the same subject
> type and orientation as the reference.
