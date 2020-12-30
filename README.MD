# Bons-OS

If you are using Windows is recommended to use the [WSL](https://docs.microsoft.com/it-it/windows/wsl/install-win10) with [Debian](https://www.microsoft.com/it-it/p/debian/9msvkqc78pk6?activetab=pivot:overviewtab) or [Ubuntu](https://www.microsoft.com/it-it/p/ubuntu/9nblggh4msv6?activetab=pivot:overviewtab) (Ubuntu may be faster). WSL does not include a graphical interface so [QEMU](https://www.qemu.org/download/#windows) must be installed to run and debug the OS. [GDB](https://www.gnu.org/software/gdb/) is used as a debugger and it can be installed with [MinGW](https://osdn.net/projects/mingw/downloads/68260/mingw-get-setup.exe/) as it contains GDB and other usefull programs. Don't forget to [add to Window's path](https://helpdeskgeek.com/windows-10/add-windows-path-environment-variable/) QEMU and GDB.

## Install needed dependencies:
Make sure to run `$ apt-get update` and `$ apt-get upgrade` before. Then run `$ ./dependencies.sh` to setup the project, it is a long process.

## Run the project:
Open the folder using [Visual Studio Code](https://code.visualstudio.com/) and install the [Native Debug extension](https://marketplace.visualstudio.com/items?itemName=webfreak.debug). On Windows, make sure that the default terminal is setted to cmd or powershell. Hit F5 to start the debugger or CTRL+SHIFT+B to build the .img file 

## Export the OS
Finally you can run the .img file in a virtual machine or burn it to a drive using a tool like [Rufus](https://rufus.ie/).

### Troubleshooting
If when hitting F5 to debug on vscode an error is prompted, try opening run.cmd and edit the third line increasing the number.
