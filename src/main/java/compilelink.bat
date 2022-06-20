@echo off
set cfile=%1
set binaryfile=%2
gcc -c -I%JAVA_HOME%\include -I%JAVA_HOME%\include\win32 %cfile%.c -o %cfile%.o -O3
gcc -shared -o %binaryfile%.dll %cfile%.o -Wl