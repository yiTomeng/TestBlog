# -*- coding: cp936 -*-
from PIL import Image
pic_path = "C:\\pytest\\1.jpg"
testim = Image.open(pic_path).convert("L") #转成灰阶
pic_w = testim.size[0]
pic_h = testim.size[1]

def split_pic(w,h,s):
    boxs = [(x,y,x+s,y+s) for x in range(0,w,s) for y in range(0,h,s)]
    for box in boxs:
        img = testim.crop(box)
        yield img,box

def avg_grey(pic):
    a = list(pic.getdata())
    return sum(a)/len(a)
def works():
    for p in split_pic(pic_w,pic_h,10):
        img,box = p
        for grey in range(0,256,41):
            if grey<=avg_grey(img)<grey+41:
                testim.paste(grey+20,box) #这里grey+20的地方可以改成图像
                break
    testim.save("C:\\pytest\\11.jpg","JPEG")
    testim.show()
works()
