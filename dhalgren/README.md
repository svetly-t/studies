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
pacman -S zip mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_ttf
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

On Intel-macs:

It's been too long, but I think you just need to `brew install sdl2 sdl2_image sdl2_ttf`?

On silicon-macs:

- Go to the SDL releases on github. (https://github.com/libsdl-org/SDL/releases)
- Find the most recent SDL2 release.
- Download `SDL2-2.**.*.zip`. As of writing the most recent version was `2.32.6`.
- Per INSTALL.txt:
```
    macOS from the command line:
    * Run './configure; make; make install'
    * (Note: I had to run 'sudo su' beforehand, otherwise permission failure)
```
- Do the same thing for SDL2_image (I used version `2.8.12`) and SDL2_ttf (`2.24.0`).


## emscripten install

Try this on WSL2, for Ubuntu 24.04. Couldn't get this working on msys:

```
sudo apt update
sudo apt install -y git cmake python3
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

Separately:

```
sudo apt update
sudo apt install zip
```