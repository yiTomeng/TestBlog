import os.path,sys
import time
#dirname=sys.argv[1]

dirname = "C:/new/"

"""
i=10001
for f in os.listdir(dirname):
    print f
    src=os.path.join(dirname,f)
    if os.path.isdir(src):
        continue
    os.rename(src,str(i))
    i+=1
"""

curtime = time.strftime("_%Y%m%d%H%M%S", time.localtime(time.time()))

for f in os.listdir(dirname):
    print f
    a = os.path.splitext(f)
    src = os.path.join(dirname,f)
    print src
    dst = a[0]+ curtime
    print dst
    dst = dst + a[1]
    print dst
    dst = os.path.join(dirname, dst)
    os.rename(src, dst)
