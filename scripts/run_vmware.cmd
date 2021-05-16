@echo off
echo make sure you have VMWare setted up correctly
echo make sure to update the path of the disk in ./vmware/BonsOS/BonsOS.*

bash -c "make"
echo Running

if exist BonsOS.hdd del BonsOS.hdd > nul
copy BonsOS.img BonsOS.hdd > nul
start vmplayer vmware\BonsOS\BonsOS.vmx
