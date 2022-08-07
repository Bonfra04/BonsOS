# BonsOS
BonsOS is an x86 only operating system. A minimal bootloader loads the kernel which sets up all the essential things and initialize the comunication between hardware and software. After the initialization process, the kernel loads the `init.elf` executable which is the root of all the processes.

## Building the OS
The build process is meant to be runned under linux, if on windows please use [WSL](https://docs.microsoft.com/it-it/windows/wsl/install-win10) (I suggest using [Ubuntu](https://www.microsoft.com/it-it/p/ubuntu/9nblggh4msv6?activetab=pivot:overviewtab) with WSL2). WSL supports GUI programs only under Windows 11, [VcXsrv](https://sourceforge.net/projects/vcxsrv/) is a good option to emulate it under Winows 10.

### Install dependencies:
You are going to need [Visual Studio Code](https://code.visualstudio.com/) to easly run/debug the project.
On Windows make sure that you are using the [Remote - WSL](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-wsl) extension to run VSCode as if it was under linux.</br>
Run `$ ./dependencies.sh` to install al the needed dependencies and build a cross-compiler (this is a long process). The script is designed to run on Debian based distros, adapt it to your distro if needed.

### Compile the project:
To compile the code and generate the `BonsOS.img` output file you can either run `$ make` or in VSCode you can run the `compile` task.

## Run the project:
You can either run it manually via any virtual machine or inside VSCode running the `run qemu (debug)` task.

### Debug the project:
Open the folder in VSCode and install the [Native Debug](https://marketplace.visualstudio.com/items?itemName=webfreak.debug) extension. Hit F5 to start the debugger.

### Troubleshooting
If when pressing F5 in VSCode qemu idle with "Guest has not initialized the display (yet)" try increasing the sleep amount in the `tasks.json` config file.

## Credits
A big portion of the code present in the bootloader is inspired (copied) by [Limine](https://github.com/limine-bootloader/limine), do not assume i know anything about that code :P
