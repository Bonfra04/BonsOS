@echo off
echo make sure you update BOCHS config file updated with the right path of the disk

bash -c "make clean; make"
echo Generating image...
bash -c "./create_image.sh >& /dev/null"
echo Running

del BonsOS.img.lock
start bochs -f bochsrc.bxrc -q