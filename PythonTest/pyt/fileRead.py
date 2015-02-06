import codecs
import binascii
#f = open("servicesituation.msg").read()
f = open("servicesituation.msg", 'r')
w = open("servicesituation1.txt", "w")

for line in f:
    #print binascii.hexlify(line)
    for i in line:
        w.write(binascii.hexlify(i))


"""
for line in open('test.txt', 'r'):
    print line
"""
