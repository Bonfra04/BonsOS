nasm -felf64 test.asm
ld -T binary.ld test.o -o test
rm test.o