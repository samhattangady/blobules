from PIL import Image

images = ['player.png', 'cube.png', 'wall.png', 'ground.png', 'slippery.png', 'hot.png', 'cold.png', 'furn.png', 'player.png']
spritesheet = Image.new('RGBA', (200*9,300), (0,0,0,0))

for i, image in enumerate(images):
    sprite = Image.open(image)
    if sprite.size[1] ==150:
        y_offset=151
    else:
        y_offset=0
    spritesheet.paste(sprite, (i*200, y_offset))

# im = Image.open('sdf_output.png')
# im = im.resize((90,180))
# spritesheet.paste(im, (0,0))
spritesheet.save('spritesheet.png')
