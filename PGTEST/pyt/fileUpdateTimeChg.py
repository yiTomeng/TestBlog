import os
import time

searchDir = "C:/fileUpdateTimeChg"

curtime = time.strftime("%Y%m%d%H%M%S", time.localtime(time.time()))

result = os.listdir(searchDir)
#print result
for num in range(len(result)):
#    print result[num]
    childResult = result[num]
    searchChild = searchDir + "/" + childResult
    if os.path.isdir(searchChild):
        #print searchChild
        fileResult = os.listdir(searchChild)
        for childNum in range(len(fileResult)):
            fileOldName = fileResult[childNum]
            fileOldPath = searchChild + "/" + fileOldName
            #print fileOldPath
            if os.path.isfile(fileOldPath):
                fileAna = os.path.splitext(fileOldName)
                filePre = fileOldName.split("_")
                #print fileAna
                #print filePre
                if len(filePre) > 2:
                    #print filePre[1]
                    fileNewName = filePre[0] + "_" + filePre[1] + "_" + curtime + fileAna[1]
                    fileNewPath = searchChild + "/" + fileNewName
                    print fileNewPath
                    #os.remove(fileOldPath)
                    #newFile = open(fileNewPath, 'w')
                    #newFile.close()
                    os.rename(fileOldPath, fileNewPath)
