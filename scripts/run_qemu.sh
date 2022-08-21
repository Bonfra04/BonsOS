if [ -z ${1+x} ]
then
    qemu-system-x86_64 -smp 1 -M q35 -m 512M -drive file=./BonsOS.img,index=0,media=disk,format=raw
elif [ $1 == "DEBUG" ]
then
    qemu-system-x86_64 -smp 1 -M q35 -m 512M -drive file=./BonsOS.img,index=0,media=disk,format=raw -no-reboot -no-shutdown -S -gdb tcp::9000 -d int -D qemu.log
else
    echo "Error"
fi