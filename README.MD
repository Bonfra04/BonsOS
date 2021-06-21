# BonsOS
BonsOS is a simple operating system. A simple home made bootloader loads the kernel which is responsible of seting up all the essential things and initialize the comunication between the software and the hardware. After the initialization process the kernel loads the `init.elf` executable which is the root of all the processes.

## Build tools
### Using windows:
Using windows [WSL](https://docs.microsoft.com/it-it/windows/wsl/install-win10) is mandatory to execute correctly the code (I suggest using [Ubuntu](https://www.microsoft.com/it-it/p/ubuntu/9nblggh4msv6?activetab=pivot:overviewtab)). WSL does not yet include a graphical interface so [Xming](https://sourceforge.net/projects/xming/files/latest/download) is a good option to emulate it.

## Install needed dependencies:
You are going to need [Visual Studio Code](https://code.visualstudio.com/) to easly run/debug the project.
On Windows make sure that you are using the [Remote - WSL](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-wsl) extension to run VSCode as if it was under linux.</br>
Run `$ apt-get update` and `$ apt-get upgrade` to ensure your system is up to date, then run `$ ./dependencies.sh` to install al the needed dependencies, it is a long process.

## Compile the project:
To compile the code and generate the `BonsOS.img` output file you can either run `$ make` or in VSCode you can run the `compile` task.

## Run the project:
Open the folder in VSCode and and run the `run qemu` task.

## Debug the project:
Open the folder in VSCode and install the [Native Debug](https://marketplace.visualstudio.com/items?itemName=webfreak.debug) extension. Hit F5 to start the debugger. Two panels will show up hosting the `qemu monitor` and the `qemu serial`; remember to close them before starting the next debug session.

## Export the OS
Finally you can run the .img file in a virtual machine or burn it to a drive using a tool like [Rufus](https://rufus.ie/) for windows.

