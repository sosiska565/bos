#asm files compile
nasm -f elf32 boot/boot.asm -o build/boot.o

#C files compile
gcc -m32 -c krnl/header.c -o build/krnl.o

#linked files
i686-elf-ld -T link.ld -o bos.bin build/boot.o build/krnl.o

#launch VM
wsl qemu-system-i386 -kernel bos.bin