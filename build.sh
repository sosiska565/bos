nasm -f elf32 sys/boot/boot.asm -o build/boot.o
gcc -m32 -c sys/kernel/header.c -o build/krnl.o
ld -m elf_i386 -T link.ld -o kernel build/boot.o build/krnl.o