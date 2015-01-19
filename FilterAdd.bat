@rem echo %cd%
@rem echo %~dp0
set now="%cd%"
@rem echo now

set batfile="%~dp0"
@rem echo %batfile%

cd %batfile%
@rem echo %cd%

@rem cd %now%
@rem echo %now% 
@rem pause
copy FLVSplitter.ax C:\WINDOWS\system32\
copy MP4Splitter.ax C:\WINDOWS\system32\
regsvr32 FLVSplitter.ax
regsvr32 MP4Splitter.ax
copy flvdecvp6.dll C:\WINDOWS\system32\
copy flvsplit.dll C:\WINDOWS\system32\

cd %now%

pause