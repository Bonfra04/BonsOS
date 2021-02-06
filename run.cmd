@echo off
bash -c "make clean; make"
echo Generating image...
bash -c "./create_image.sh >& /dev/null"
echo Running
start qemu-system-x86_64 -m 512M -M q35 -hda BonsOS.img -no-reboot -no-shutdown -S -gdb tcp::9000
timeout 1 /nobreak