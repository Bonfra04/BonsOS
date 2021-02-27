#mkdir
mkdir bin/img
rm -f BonsOS.img

#Create the partition
dd if=/dev/zero of=./bin/img/partition.dd bs=512 count=65536 # count = [ K = megabyte; K*(1024)^2/512 ]
mkfs.vfat -F 16 -n "BonsOS" ./bin/img/partition.dd

#Add file to the partition
mcopy -i ./bin/img/partition.dd ./bin/boot/loader.bin ::/
mcopy -i ./bin/img/partition.dd ./bin/kernel/kernel.sys ::/

#mmd -i ./bin/img/partition.dd ::/folder
#mcopy -i ./bin/img/partition.dd ./test.txt ::/

#Add the bootloader to the partition
dd if=bin/boot/boot.bin of=./bin/img/partition.dd seek=0 count=1 conv=notrunc bs=3
dd if=bin/boot/boot.bin of=./bin/img/partition.dd seek=90 skip=90 count=$[512-90] conv=notrunc bs=1

#Create the Disk image
dd if=/dev/zero of=BonsOS.img bs=512 count=$[2048+65536]
echo -e "n \n p \n \n \n \n t \n 6\n a \n w" | fdisk BonsOS.img

#Load the MBR 
dd if=bin/boot/mbr.bin of=BonsOS.img seek=0 count=1 conv=notrunc bs=436

#Add the partition to the disk
dd if=./bin/img/partition.dd of=BonsOS.img conv=notrunc bs=512 seek=2048
