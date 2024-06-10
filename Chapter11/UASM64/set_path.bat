set UASM_PATH=%~dp0

%~d0
cd  %UASM_PATH%

set path=;%UASM_PATH%bin;%path%
set include=%UASM_PATH%include
set lib=%UASM_PATH%lib
