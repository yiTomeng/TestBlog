@echo off
echo.
echo =================================
echo �t�@�C���������l�[�����Ă��܂�...
echo =================================
echo.
rem �o�b�`������f�B���N�g���ֈړ�
pushd %~dp0
rem �t�@�C�������ꊇ���l�[������
for %%f in ( * ) do call :sub "%%f"
exit /b
:sub
set filename=%1
set filename=%filename:20140903_=%
ren %1 %filename%
goto :EOF
exit