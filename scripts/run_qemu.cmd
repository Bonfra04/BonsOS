@echo off
bash -c "make"
echo Running

start qemu-system-x86_64 -M q35 -m 512M -drive file=./BonsOS.img,index=0,media=disk,format=raw -no-reboot -no-shutdown -S -gdb tcp::9000 -monitor stdio -d int -D qemu.log

timeout 3
