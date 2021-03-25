@echo off
echo make sure you have VMWare setted up correctly
echo make sure to update the path of the disk in ./vmware/BonsOS/BonsOS.*

bash -c "make clean; make"
echo Generating image...
bash -c "./create_image.sh >& /dev/null"
echo Running

del BonsOS.hdd
copy BonsOS.img BonsOS.hdd
start vmplayer vmware\BonsOS\BonsOS.vmx