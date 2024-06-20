cmake -S . -B build -G "MinGW Makefiles" .
mingw32-make.exe -C build
.\build\rt-renderer\rt-renderer.exe