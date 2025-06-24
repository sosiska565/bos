export PREFIX="$HOME/os" # Папка, где будут бинари.
export TARGET=i686-elf # У нас будет i686 процессор, и компилятор генерирует ELF файлы.
export PATH="$PREFIX/bin:$PATH"

cd $HOME/cross-src # Папка с сорсами для Binutils & GCC.
mkdir build-binutils
cd build-binutils
../binutils-2.24/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install