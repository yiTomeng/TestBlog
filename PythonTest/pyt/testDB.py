# -*- coding: utf-8 -*-
import psycopg2
#import codecs
import types 
"""
import sys
import locale


def p(f):
    print '%s.%s(): %s' % (f.__module__, f.__name__, f())
 
# 返回当前系统所使用的默认字符编码
p(sys.getdefaultencoding)
 
# 返回用于转换Unicode文件名至系统文件名所使用的编码
p(sys.getfilesystemencoding)
 
# 获取默认的区域设置并返回元祖(语言, 编码)
p(locale.getdefaultlocale)
 
# 返回用户设定的文本数据编码
# 文档提到this function only returns a guess
p(locale.getpreferredencoding)
"""

conn = psycopg2.connect(database="Operation information", user="admin", password="signal", host="192.168.1.22", port="5432")


cur = conn.cursor()

pass_setting_file = open("C:/test/d_basic_attendant_change_setting.txt", 'r')

##cur.execute("SET NAMES sjis;")
#cur.execute("set client_encoding to sjis;")

#cur.execute("SELECT * FROM unity.m_rosen_section;")
#print pass_setting_file.read()
m  = pass_setting_file.readlines()
for i in m:
#    print i
    if i == "":
        print "blank"
        continue
    else:
        print i
        cur.execute(i)

#rows = cur.fetchall()        # all rows in table

#print(rows)

"""
for i in rows:
    ##print repr(i.decode('mbcs'))
    print i
    #print i[0]
    #print i[1].decode('mbcs').encode('utf8')
    for m in i:
        if type(m) == types.StringType:
            #print m.decode('mbcs').encode('utf8')
            print m
        else:
            print m
"""
