/*
 * Delaford sprite capture — paste into the ChatGPT tab's DevTools Console (F12).
 * Prereq: run  python receive_images.py  in the sprite_samples folder first.
 *
 * Grabs the most recent large image in the conversation, converts it to base64,
 * and POSTs it to the local capture server, which saves it into sprite_samples/.
 * Tip: click the generated image to open it full-size first for max resolution.
 */
(async () => {
  // ===== EDIT per sprite =====
  const FILENAME = "weapon_generated.png";   // weapon / armor / terrain ..._generated.png
  const SERVER   = "http://localhost:8765";  // try http://127.0.0.1:8765 if this is blocked
  // ===========================

  const imgs = [...document.querySelectorAll("img")]
    .filter(im => (im.naturalWidth || im.width) >= 256 &&
                  (im.naturalHeight || im.height) >= 256);
  if (!imgs.length) {
    console.error("[capture] No image >=256px found. Make the generated image visible (or open it full-size).");
    return;
  }
  // Prefer the largest; tie-break to latest in DOM order.
  const img = imgs.reduce((a, b) =>
    (b.naturalWidth * b.naturalHeight) >= (a.naturalWidth * a.naturalHeight) ? b : a);
  console.log(`[capture] Using ${img.naturalWidth}x${img.naturalHeight} image:`, img.src.slice(0, 90));

  let dataUrl;
  try {
    const blob = await (await fetch(img.src)).blob();
    dataUrl = await new Promise((res, rej) => {
      const r = new FileReader();
      r.onload = () => res(r.result);
      r.onerror = rej;
      r.readAsDataURL(blob);
    });
  } catch (e) {
    console.error("[capture] Could not read image bytes (CORS?). Try opening the image full-size, then rerun.", e);
    return;
  }

  try {
    const resp = await fetch(SERVER, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ name: FILENAME, data: dataUrl }),
    });
    const j = await resp.json();
    console.log("[capture] ✅ Saved ->", j.path);
  } catch (e) {
    console.error("[capture] POST failed. Is receive_images.py running? Try 127.0.0.1 instead of localhost.", e);
  }
})();
