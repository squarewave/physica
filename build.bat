@echo off

set CommonCompilerFlags=-Zi -O2 /I "C:\common\include"
set CommonLinkerFlags= -libpath:"C:\common\lib\SDL2" -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib SDL2.lib

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2> NUL

REM 64-bit build
cl %CommonCompilerFlags% -DTRANSLATION_UNIT_INDEX=2 ..\main.cpp -Fmwin32_physica.map /link %CommonLinkerFlags%
popd
