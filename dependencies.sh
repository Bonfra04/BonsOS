ROOT_DIR=$(pwd)

sudo apt-get update
sudo apt-get upgrade

# Packages:
sudo apt-get install --assume-yes make
sudo apt-get install --assume-yes gcc
sudo apt-get install --assume-yes g++
sudo apt-get install --assume-yes binutils
sudo apt-get install --assume-yes make
sudo apt-get install --assume-yes bison
sudo apt-get install --assume-yes flex
sudo apt-get install --assume-yes libgmp3-dev
sudo apt-get install --assume-yes libmpc-dev
sudo apt-get install --assume-yes libmpfr-dev
sudo apt-get install --assume-yes texinfo
sudo apt-get install --assume-yes libcloog-isl-dev
sudo apt-get install --assume-yes libisl-dev
sudo apt-get install --assume-yes curl
sudo apt-get install --assume-yes nasm
sudo apt-get install --assume-yes dosfstools
sudo apt-get install --assume-yes mtools
sudo apt-get install --assume-yes qemu
sudo apt-get install --assume-yes qemu-kvm
sudo apt-get install --assume-yes gdb

mkdir $ROOT_DIR/tmp/
mkdir $ROOT_DIR/tools/

# Cross Compiler:

curl -o $ROOT_DIR/tmp/binutils-2.35.tar.gz 'https://ftp.gnu.org/gnu/binutils/binutils-2.35.tar.gz'
curl -o $ROOT_DIR/tmp/gcc-10.2.0.tar.gz 'https://ftp.gnu.org/gnu/gcc/gcc-10.2.0/gcc-10.2.0.tar.gz'
tar -xvf $ROOT_DIR/tmp/binutils-2.35.tar.gz -C $ROOT_DIR/tmp/
tar -xvf $ROOT_DIR/tmp/gcc-10.2.0.tar.gz -C $ROOT_DIR/tmp/

export PREFIX="$ROOT_DIR/tools/cross-compiler"
export TARGET=x86_64-elf
export PATH="$PREFIX/tools/cross-compiler:$PATH"

cd $ROOT_DIR/tmp/
mkdir build-binutils
cd build-binutils
$ROOT_DIR/tmp/binutils-2.35/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

cd $ROOT_DIR/tmp/
which -- $TARGET-as || echo $TARGET-as is not in the PATH
mkdir build-gcc
cd build-gcc
$ROOT_DIR/tmp/gcc-10.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

# Clean
rm -dr $ROOT_DIR/tmp/