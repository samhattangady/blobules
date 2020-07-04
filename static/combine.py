from PIL import Image

images = ['cube.png', 'wall.png', 'ground.png', 'hot.png', 'cold.png', 'furn.png', 'player.png']
for i in range(30):
    images.append(f'player_{i}.png')
for i in range(16):
    images.append(f'slippery_{i}.png')

spritesheet = Image.new('RGBA', (200*len(images),300), (0,0,0,0))

for i, image in enumerate(images):
    print(f'adding {image}')
    sprite = Image.open(image)
    if sprite.size[1] ==150 or 'slippery' in image:
        sprite=sprite.resize((200, 300))
    #     if sprite.size[1] == 300:
    #         sprite = sprite.resize((200, 150))
    #     y_offset=150
    # else:
    #     y_offset=0
    spritesheet.paste(sprite, (i*200, 0))

# im = Image.open('sdf_output.png')
# im = im.resize((90,180))
# spritesheet.paste(im, (0,0))
spritesheet.save('spritesheet.png')
