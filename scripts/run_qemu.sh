make
echo Running

setsid qemu-system-x86_64 -M q35 -m 512M -hda ./BonsOS.img -no-reboot -no-shutdown -S -gdb tcp::9000 &>/dev/null </dev/null & disown
