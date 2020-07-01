from krita import *
import os
import pathlib
#import Pillow

dirpath = pathlib.Path(__file__).parent.absolute() 
export_all = False
export_slippery=False
export_player = True

if export_all:
    objects = ['player', 'cube', 'furn', 'ground', 'slippery', 'cold', 'hot', 'wall']
    for obj in objects:
        print(obj)
        doc = Krita.instance().openDocument(os.path.join(dirpath, f'{obj}.kra'))
        doc.setBatchmode(True)
        background = doc.nodeByName('Background')
        if background:
            background.remove()
            doc.save()
        doc.exportImage(os.path.join(dirpath, f'{obj}.png'), InfoObject())

if export_slippery:
    doc = Krita.instance().openDocument(os.path.join(dirpath, 'slippery.kra'))
    doc.setBatchmode(True)
    current_frame = doc.fullClipRangeStartTime()
    last_frame = doc.fullClipRangeEndTime()
    while current_frame <= last_frame:
        print(f'current_frame = {current_frame}')
        doc.setCurrentTime(current_frame)
        doc.exportImage(os.path.join(dirpath, f'slippery_{current_frame}.png'), InfoObject())
        current_frame += 1
 
if export_player:
    doc = Krita.instance().openDocument(os.path.join(dirpath, 'player.kra'))
    doc.setBatchmode(True)
    current_frame = doc.fullClipRangeStartTime()
    last_frame = doc.fullClipRangeEndTime()
    while current_frame <= last_frame:
        print(f'current_frame = {current_frame}')
        doc.setCurrentTime(current_frame)
        doc.exportImage(os.path.join(dirpath, f'player_{current_frame}.png'), InfoObject())
        current_frame += 1
