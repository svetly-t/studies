## Steps for building on windows

Download vscode.

Download msys2: https://www.msys2.org/

Download git bash (for making commits): https://git-scm.com/downloads/win

Open this project in vscode. Press the arrow next to the plus sign in vscode and press "Select default profile." Change it to "bash (MSYS2)".

In the terminal, download the requisite toolchain: `pacman -S --needed base-devel mingw-w64-x86_64-toolchain`. See https://superuser.com/questions/1718287/cannot-find-g-after-msys2-install

Append the mingw64 bin directory to your PATH: 
```
echo 'export PATH=$PATH:/mingw64/bin' >> ~/.bashrc
tail ~/.bashrc
source ~/.bashrc
```

Download SDL2 as a package within msys2: https://packages.msys2.org/packages/mingw-w64-x86_64-SDL2
```
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image
```

To build and run:
```
make win
./kid.exe
```

`kid.exe` must be run from within the msys2 shell. Otherwise Windows can't find the libstdc++ or SDL2 DLLs. This is a future problem!