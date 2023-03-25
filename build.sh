set -eu
set -o pipefail

BUILDDIR=build
rm -rf $BUILDDIR
mkdir $BUILDDIR

nasm -f elf64 crtstub.S -o $BUILDDIR/crtstub.o
clang -o $BUILDDIR/main.o -fpie -nostdlib -fno-stack-protector -fno-finite-loops -fwrapv -fno-strict-aliasing -static -Os -c main.c
ld -o xasm -T link.ld --gc-sections $BUILDDIR/crtstub.o $BUILDDIR/main.o
# strip -s xasm
