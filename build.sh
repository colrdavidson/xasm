set -eu
set -o pipefail

build_linux() {
	nasm -f elf64 crtstub.S -o $BUILDDIR/crtstub.o
	clang -o $BUILDDIR/main.o -fpie -nostdlib -fno-stack-protector -fno-finite-loops -fwrapv -fno-strict-aliasing -static -Os -c main.c
	ld -o xasm -T link.ld --gc-sections $BUILDDIR/crtstub.o $BUILDDIR/main.o
	# strip -s xasm
}

build_darwin() {
	clang main.c -o xasm -fno-finite-loops -fwrapv -fno-strict-aliasing -Os
}

BUILDDIR=build
rm -rf $BUILDDIR
mkdir $BUILDDIR

OS=$(uname)
case $OS in
Linux)
	build_linux
	;;
Darwin)
	build_darwin
	;;
*)
	panic "Platform unsupported!"
esac
