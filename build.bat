@echo off

echo "hi"

rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set CommonCompilerFlags=-Zi -Od /I "C:\common\include"
set CommonLinkerFlags= -libpath:"C:\common\lib" -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib SDL2main.lib SDL2.lib openGL32.lib glew32.lib

echo "hi"
node build_animations.js

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2> NUL

REM 64-bit build
cl %CommonCompilerFlags% -DTRANSLATION_UNIT_INDEX=2 ..\main.cpp -Fmwin32_physica.map /link %CommonLinkerFlags%
if %errorlevel% neq 0 goto :error
popd
goto :EOF

:error
popd
exit /b %errorlevel%
