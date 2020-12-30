@echo off
bash -c "make clean; make"
echo Generating image...
bash -c "./create_image.sh >& /dev/null"
echo Running
start qemu-system-x86_64 -hda BonsOS.img -m 512M -no-reboot -no-shutdown -S -gdb tcp::9000
timeout 1 /nobreak