RISCV=/opt/riscv
TARGET=riscv64-unknown-elf

CC=${RISCV}/bin/${TARGET}-gcc -march=rv32im -mabi=ilp32
CXX=${RISCV}/bin/${TARGET}-g++ -march=rv32im -mabi=ilp32
AS=${RISCV}/bin/${TARGET}-as
LD=${RISCV}/bin/${TARGET}-ld
STRIP=${RISCV}/bin/${TARGET}-strip
GDB=${RISCV}/bin/${TARGET}-gdb
PK=${RISCV}/${TARGET}/bin/pk
DUMP=${RISCV}/${TARGET}/bin/objdump
READELF=${RISCV}/bin/${TARGET}-readelf

QEMU=qemu-riscv32

MAK=make --no-print-directory

CXXFLAGS=-std=c++0x -Wall -pedantic
GXXFLAGS=-g ${CXXFLAGS}

GFLAGS=-g -Wall -pedantic -std=c99
OFLAGS=-O -Wall -pedantic -std=c99
CFLAGS=${OFLAGS}
LDFLAGS=
LDLIBS=-lm

XTRA=

all:
	less README.md

%.gcc: %.c
	gcc -o $@ $< -lelf

%.s: %.c
	${CC} -S ${OFLAGS} $<

%.o: %.S
	${CC} -c $<

%.o: %.s
	${CC} -c $<

%.qemu-O: %.o
	${CC} ${OFLAGS} ${LDFLAGS} $(*F).o ${LDLIBS} -o $@

%.dump-qemu: %.qemu-O
	${DUMP} -s $< > objdump_$<.txt

%.readelf-qemu: %.qemu-O
	${READELF} -a $< > readelf_$<.txt

# /opt/riscv/bin/riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -O -Wall -pedantic -std=c99   -c -o fact10.o fact10.c
# /opt/riscv/bin/riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -O -Wall -pedantic -std=c99  -nostartfiles fact10.o term.o crt0.o  -lm -o fact10.lrv
# /opt/riscv/bin/riscv64-unknown-elf-readelf -a fact10.lrv | grep " _start"` > fact10.ram

clean:
	rm -f *.o *.ram ucode-up3 *.lrv *.qemu-O *.qemu-g
	rm -rf obj_dir
	rm -f *.vcd tb