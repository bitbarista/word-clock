# usage: python3 make_icons.py <ghost-artwork.png>
import sys
import os; OUT = os.path.dirname(os.path.abspath(__file__)) + '/'
from PIL import Image
# 16x16: the clock's own GHOST14 sprite with clock-face eyes, plus a dark outline
G = ["....BBBBBB....",
     "..BBBBBBBBBB..",
     ".BHBBBBBBBBBB.",
     ".BWWWWBBWWWWB.",
     "BBYWWYBBYWWYBB",
     "BBWYYWBBWYYWBB",
     "BBWWWWBBWWWWBB",
     "BBBBBBBBBBBBBB",
     "BBBBBBBBBBBBBB",
     "BBBBBBBBBBBBBB",
     "BBBBBBBBBBBBBB",
     "BBBBBBBBBBBBBB",
     "BBBBBBBBBBBBBB",
     "BB.BBB..BBB.BB"]
COL = {'B':(0x03,0xdd,0xf7),'W':(0xfa,0xf9,0xfb),'Y':(0xfb,0xc6,0x0a),'K':(0x16,0x14,0x16),'H':(0x8a,0xee,0xfb)}
grid = [['.']*16 for _ in range(16)]
for r,row in enumerate(G):
    for c,ch in enumerate(row): grid[r+1][c+1] = ch
out = [row[:] for row in grid]
for r in range(16):
    for c in range(16):
        if grid[r][c] != '.': continue
        if any(0<=r+dr<16 and 0<=c+dc<16 and grid[r+dr][c+dc] not in '.'
               for dr,dc in ((1,0),(-1,0),(0,1),(0,-1))): out[r][c] = 'K'
im = Image.new('RGBA',(16,16),(0,0,0,0))
for r in range(16):
    for c in range(16):
        if out[r][c] != '.': im.putpixel((c,r), COL[out[r][c]]+(255,))
im.save(OUT+'favicon-16.png', optimize=True)

# 32 and 180 from the supplied artwork
src = Image.open(sys.argv[1])  # the full-size ghost artwork.convert('RGBA')
x0,y0,x1,y1 = src.getchannel('A').point(lambda a: 255 if a>40 else 0).getbbox()
s = max(x1-x0, y1-y0); cx,cy = (x0+x1)//2,(y0+y1)//2
def square(pad):
    h = s//2 + pad
    return src.crop((cx-h,cy-h,cx+h,cy+h))
square(10).resize((32,32), Image.LANCZOS).save(OUT+'favicon-32.png', optimize=True)
# home-screen icon: opaque dark tile (iOS fills transparency with black anyway), some margin
tile = Image.new('RGBA',(180,180),(0x07,0x08,0x0b,255))
g = square(int(s*0.12)).resize((180,180), Image.LANCZOS)
tile.alpha_composite(g)
tile.convert('RGB').quantize(64).save(OUT+'apple-touch-icon.png', optimize=True)
