
# coding: Shift_JIS
 
import glob
 
# パス内の全ての"指定パス+ファイル名"と"指定パス+ディレクトリ名"を要素とするリストを返す
files = glob.glob('C:\GWServer\FTP\BasicAttendantHolidayRun\BasicAttendant_14_*.*') # ワイルドカードが使用可能
 
for file in files:
    print file
