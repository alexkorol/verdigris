from PIL import Image
from pathlib import Path
p=Path('native/client/assets/first-slice');o=p/'starter-projection';o.mkdir(exist_ok=True)
im=Image.open(p/'starter-v2/originals/player-unarmed-idle.png').convert('RGBA')
print(im.getextrema())
bg=Image.new('RGBA',im.size,(44,48,46,255));bg.alpha_composite(im);bg.save(o/'source-visible.png')
