qemu_command="qemu-system-x86_64 -smp 1 -M q35 -m 512M
-nic none
-drive file=./BonsOS.img,index=0,media=disk,format=raw
-drive if=none,id=stick,format=raw,file=./stick.img
-device piix3-usb-uhci,id=usbport
-device usb-storage,bus=usbport.0,drive=stick
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