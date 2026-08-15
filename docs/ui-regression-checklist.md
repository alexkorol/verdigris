# UI Regression Checklist

## Movement Smoothing
- Log into a world, hold `W` then change direction diagonally and confirm transitions stay eased with no snapping.
- Compare the move duration to `DEFAULT_MOVE_DURATION_MS` by counting tiles travelled; ensure long strides remain proportional.
- Watch NPC patrols for hitching after resizing the window; interpolation should remain smooth at 60 FPS.

## Pane Mechanics
- Desktop >= 1200px: click the HP orb to open Character and the MP orb to open Inventory. Both are fixed overlays: the world canvas must remain exactly the same size behind them.
- Inventory must show the complete 12×7 backpack at the authored 54px cell scale, with item art filling its footprint rather than a tiny 32px sprite.
- Close each pane with its `×`, `Esc`, and backdrop. Focus must return to the game; a second `Esc` opens the game menu.
- Tablet and mobile: open Character/Inventory and confirm every panel edge remains inside the viewport without shrinking or horizontally scrolling the game canvas.

## Chronicle / World Meta Seam
- Enter through the normal `Play as Guest` → Chronicles → `Set Out` path, not only `?play`.
- Confirm the morning wagon-purse message appears on the first set-out of the day and the Scion spawns beside its House wagon.
- Open `Roads`, choose a road, and verify the House chart opens instead of “Only a sworn scion of a House can read its chart.”
- The chart heading must contain exactly one `House` prefix. Enter a charted node and confirm the destination name replaces Delaford Village on the minimap.
- Open Character and the Skill Tree; the earned-point total must agree in both places (a fresh level-1 Scion currently has 2 of the 140 lifetime points).

## Chat + Quickbar
- With chat collapsed, send/receive a message; badge increments, preview updates, and `Show chat` opens the overlay without shifting the canvas.
- Keep chat unpinned, interact, and verify it auto-collapses after ~8s of inactivity; pinning disables auto-hide.
- Press numeric keys `1-8` while the canvas is focused; the matching quick slot highlights briefly and activates its assigned skill.
- Hit `/` to open chat and focus the input, then `Esc` to collapse (when unpinned) without affecting panes.

## Responsive Behaviour
- Sweep viewport widths 480px to 1920px ensuring:
  - Canvas scales smoothly while preserving a 16:10 aspect ratio; horizontal scroll never appears.
  - Pixel edges remain crisp at 2x scale; no browser smoothing or stretching artifacts when resizing.
  - Quickbar stays anchored between orbs; chat toggle relocates (fixed) on mobile.
  - Panes remain fixed overlays at every breakpoint, with safe tap targets and no canvas resize.
- Mobile (<768px) landscape: open chat; overlay behaves like a bottom sheet and world remains scrollable.

## Pane Scroll Checks
- Populate Inventories (use bank debug) and confirm overflow areas support native scroll momentum on all breakpoints.
- Switch between Stats <-> Inventory; the previously visited pane should restore its scroll offset and focus the first control when reopened.
