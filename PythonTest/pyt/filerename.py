import sys
import os


path = "c://abc"
filenames = os.listdir("c://abc")

for filename in filenames:
    filename = path + "//" + filename
    file_names = os.listdir(filename)
    for file_name in file_names:
        prefix =  os.path.splitext(file_name)
        prefix1 = prefix[0].split("_")
        old_file = filename + "//" + file_name
        new_name = filename + "//" + prefix1[0] + "_" + prefix1[1] + prefix[1]
        os.rename(old_file, new_name)
        ##print old_file
        ##print new_name
    
