from PIL import Image

images = ['player.png', 'cube.png', 'wall.png', 'ground.png', 'slippery.png', 'hot_target.png', 'target.png', 'furn.png', 'reflector.png']
spritesheet = Image.new('RGBA', (720,180), (0,0,0,0))

for i, image in enumerate(images):
    sprite = Image.open(image)
    if sprite.size[1] ==90:
        y_offset=90
    else:
        y_offset=0
    if image in ['hot_target.png', 'target.png']:
        tmp = Image.new('RGBA', (90, 180), (0,0,0,0))
        ground = Image.open('ground.png')
        tmp.paste(ground, (0, 90))
        sprite = Image.alpha_composite(tmp, sprite)
    spritesheet.paste(sprite, (i*90, y_offset))

spritesheet.save('spritesheet.png')
