import psycopg2
# コネクション作成
conn = psycopg2.connect("dbname=qurestamp host=192.168.33.10 user=qurestamp_adm01 password=qurestamp_adm01")
# カーソル作成
cur = conn.cursor()

# SQLコマンド実行 
column1="GRP00000000007"
column3="1"
column4="1"
column5="1"
column6="0"
column8="20150716170400"
column9="GRP00000000007"
column10="aa"
column11=column10
column12=column8
column13=column1
column14=column10
column15=column10
for i in range(1, 10):
    column2= "00" + str(i)
    column2=column2[-2:]
    print column2
    column7= "000" + str(i)
    column7=column7[-3:]
    cur.execute("INSERT INTO qurestamp.public.m_appointmentlist VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, null)",
                (column1, column2,column3,column4,column5,column6,column7,column8,column9,column10,column11,column12,column13,column14,column15))

cur.execute("INSERT INTO qurestamp.public.m_clinic VALUES ('GRP00000000007', '0   ', '0 ', '0 ', '0003', '1', 'aaa', 'aa', 'aa', 'aa', '234-2222', '123-2222-2222', 'afa@kochi-it.co.jp', 'afa@kochi-it.co.jp', '20150716170200', null, '0', '0', null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, '1', null, null, null, null, null)")

conn.commit()
# クローズ
cur.close()
conn.close()
