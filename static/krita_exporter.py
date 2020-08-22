from krita import *
import os
import pathlib

dirpath = pathlib.Path(__file__).parent.absolute() 
required_layers = ['Line', 'Shadow', 'Fill']
files = ['player', 'wall', 'ground', 'slippery', 'target', 'target2', 'cube', 'furn', 'ground1', 'ground2', 'ground3', 'ground4', 'ice1', 'ice2', 'ice3']
files = ['ground4', 'ice1', 'ice2', 'ice3']


def all_layers_present(doc):
    layers = []
    nodes = doc.topLevelNodes()
    for node in nodes:
        layers.append(node.name())
    for rl in required_layers:
        if rl not in layers:
            return False
    return True

def set_other_layers_transparent(doc, layer):
    print('setting transparetn')
    nodes = doc.topLevelNodes()
    for node in nodes:
        while node.visible():
            node.setVisible(False)
            node.setOpacity(255)
    node = doc.nodeByName(layer)
    while not node.visible():
        node.setVisible(True)
    print(f'{node.name()} visibility is {node.visible()}')

def save_all_layers(doc, name):
    if not all_layers_present(doc):
        print(f'{f} does not have all the required layers.')
        return
    doc.setBatchmode(True)
    print("set batchmode")
    for rl in required_layers:
        fname = os.path.join(dirpath, f'{name}_{rl}.png')
        if os.path.exists(fname):
            os.remove(fname)
        set_other_layers_transparent(doc, rl)
        doc.save()
        doc.refreshProjection()
        while not os.path.exists(fname):
            doc.exportImage(fname, InfoObject())
        print(f"exported image {fname}")

def main():
    for f in files:
        print('\n')
        print("get krita")
        doc = Krita.instance().openDocument(os.path.join(dirpath, f'{f}.kra'))
        print("open doc")
        Krita.instance().activeWindow().addView(doc)
        # Krita.instance().action('export_layers').trigger()
        print("set active doc")
        save_all_layers(doc, f)
        doc.close()

'''
export_all = False
export_slippery=False
export_player = False
export_main_menu = True

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

if export_main_menu:
    doc = Krita.instance().openDocument(os.path.join(dirpath, 'main_menu_anim.kra'))
    doc.setBatchmode(True)
    current_frame = doc.fullClipRangeStartTime()
    last_frame = doc.fullClipRangeEndTime()
    while current_frame <= last_frame:
        print(f'current_frame = {current_frame}')
        doc.setCurrentTime(current_frame)
        fname = os.path.join(dirpath, f'shadow_{current_frame}.png')
        doc.exportImage(fname, InfoObject())
        if os.path.exists(fname):
            current_frame += 1
'''
