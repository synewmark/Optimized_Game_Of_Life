
set cfile=%1
set binaryfile=%2
g++ -c -g -I%JAVA_HOME%\include -I%JAVA_HOME%\include\win32 -I"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\include" c_code\%cfile%.cpp -o binary\%cfile%.o -Wall -Wextra -pedantic -O3
g++ -shared -o binary\%binaryfile%.dll binary\%cfile%.o -Wl -pthread
del binary\%cfile%.o