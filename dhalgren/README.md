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

For intellisense support, your `.vscode/c_cpp_properties.json` should look something like this:
```
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**",
                "\"C:\\msys64\\mingw64\\include\\**\""
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "compilerPath": "C:/msys64/mingw64/bin/g++.exe",
            "intelliSenseMode": "windows-gcc-x64"
        }
    ],
    "version": 4
}
```

## Steps for mac

It's been too long, but I think you just need to `brew install sdl2 sdl2_image sdl2_ttf`?