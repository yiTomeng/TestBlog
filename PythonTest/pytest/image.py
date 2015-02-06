from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
#im = Image.open(r"C:\pytest\1.jpg").convert("L")
im = Image.open(r"C:\pytest\1.jpg")
# Creates an object that can be used to draw in the given image.
draw = ImageDraw.Draw(im)
# draw.line(xy, options) => Draws a line between the coordinates in the xy list.
# The coordinate list can be any sequence object containing either 2-tuples [ (x, y), ... ]
# or numeric values [ x, y, ... ].
# The fill option gives the color to use for the line.
draw.line((0, 0) + im.size, fill=128)
draw.line((0, im.size[1], im.size[0], 0), fill=128)

draw.line((im.size[0]-50, 0, im.size[0]-50, 50), fill=128)
draw.line((im.size[0]-50, 50, im.size[0], 50), fill=128)
draw.line((im.size[0]-30, 0, im.size[0] - 30, 100), fill=128)

#draw.
font = ImageFont.truetype('simsun.ttc',50)
#font = ImageFont.truetype("TIVERTON.TTF", "18")
#font_width, font_height = font.getsize("5")
width, height = im.size
draw.text(((width - 50), (height - 50)),"5", font=font, fill="red")
del draw
#print im.size

# write to stdout
im.save("C:\\pytest\\4", "PNG")
#im.show()
