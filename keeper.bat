@echo off
set _task=CameraClient.exe
set _path=D:\chenlin_work\project\C++\CameraClient01\CameraClient\x64\Debug
goto start 
:start
tasklist -v | findstr /i "%_task%" > NUL  
if ErrorLevel 1 (
 
 start /d "%_path%" %_task% 
 echo ���� %_task% ������,������
) else ( 
 echo ���� %_task% ���� 
)
echo Wscript.Sleep WScript.Arguments(0) >%tmp%\delay.vbs 
cscript //b //nologo %tmp%\delay.vbs 10000 
goto start