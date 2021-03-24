@echo off
echo make sure you have VirtualBox setted up correctly
echo make sure to update the path of the disk in ./vbox/BonsOS/BonsOS.vbox

bash -c "make clean; make"
echo Generating image...
bash -c "./create_image.sh >& /dev/null"
echo Running

del BonsOS.hdd
copy BonsOS.img BonsOS.hdd
start VirtualBoxVM --startvm BonsOS --debug-command-line