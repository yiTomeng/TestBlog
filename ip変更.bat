@rem set IP_INPUT_MODE =

@rem echo 
set /P IP_INPUT_MODE="IP�@�A�h���X�̎O�ڂ���͂��Ă��������F"

@rem echo %IP_INPUT_MODE%
@rem set /P close="����:(Y/N)?"

if "%IP_INPUT_MODE%"=="3" (
@echo %IP_INPUT_MODE%
netsh interface ip set address "���[�J�� �G���A�ڑ�" static 192.168.3.240 255.255.255.0 
@rem set /P close = "3:"
) else (
@echo %IP_INPUT_MODE%
netsh interface ip set address "���[�J�� �G���A�ڑ�" static 192.168.1.240 255.255.255.0 

@rem set /P close = "1:"
)

pause
@rem set /P close = "����:(Y/N)?"