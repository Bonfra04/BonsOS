qemu_command="qemu-system-x86_64 -smp 1 -M q35 -m 512M
-drive file=./BonsOS.img,index=0,media=disk,format=raw
-nic none
-drive if=none,id=stick,format=raw,file=./stick.img
"

if [ -z ${1+x} ]
then
    $qemu_command
elif [ $1 == "DEBUG" ]
then
    $qemu_command -no-reboot -no-shutdown -S -gdb tcp::9000 -d int -D qemu.log
else
    echo "Error"
fi