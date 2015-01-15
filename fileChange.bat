@echo off
echo.
echo =================================
echo ファイル名をリネームしています...
echo =================================
echo.
rem バッチがあるディレクトリへ移動
pushd %~dp0
rem ファイル名を一括リネームする
for %%f in ( * ) do call :sub "%%f"
exit /b
:sub
set filename=%1
set filename=%filename:20140903_=%
ren %1 %filename%
goto :EOF
exit