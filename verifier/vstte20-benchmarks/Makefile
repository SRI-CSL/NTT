#------------------------------------------------------------------#
# Usage:
#   make all
#   make install INSTALL_DIR=XXX
#------------------------------------------------------------------#

#
# -D_BSD_SOURCE is needed on Linux otherwise random()
# is not declared in <stdlib.h>
#

CC=gclang
LINKER=llvm-link
INSTALL=install
INSTALL_DIR=.

PROCESSOR=$(shell uname -p)
OS=$(shell uname -s)

#CPPFLAGS=-DNDEBUG -D_BSD_SOURCE -DNDEBUG 
CFLAGS=-Wall -std=c99 -Wno-nullability-completeness
#export CGO_CPPFLAGS="-Wno-error -Wno-nullability-completeness -Wno-expansion-to-defined"
# Optimization level has a big impact on Clam.
# Disable here optimization. Clam has its own version of the LLVM
# optimization "opt".
#CFLAGS += -O1
CFLAGS += -m64 
ifeq ($(PROCESSOR), arm)
ifeq ($(OS), Darwin)
CFLAGS += -arch arm64
# To find -lSystem
LDFLAGS += -L $(shell xcrun --show-sdk-path)/usr/lib
endif 
endif
CFLAGS += -O0 -Xclang -disable-O0-optnone # avoid add optnone
# Clam does not like vector operations
CFLAGS += -fno-vectorize -fno-slp-vectorize

# To include unrolling pragmas for clang
#CFLAGS += -DUNROLLING_NTT_1024
# To include assertions that should be proven by Clam
CFLAGS += -DSAFE
# To replace call to red with a crab intrinsic
CFLAGS += -DUSE_CRAB_INTRINSICS

GET_BC=get-bc

all: intt_red1024 intt_red1024b ntt_red1024 ntt_red1024b ntt_red1024c ntt_red1024d ntt_red1024e ntt_red1024f


intt_red1024: ntt_red1024_tables.o ntt_red.o 
	$(CC) ${CFLAGS} -c  harness_intt_red1024.c 
	$(GET_BC) harness_intt_red1024.o
	$(GET_BC) ntt_red1024_tables.o
	$(GET_BC) ntt_red.o
	$(LINKER) --override harness_intt_red1024.o.bc \
		ntt_red1024_tables.o.bc \
		ntt_red.o.bc -o harness_intt_red1024.all_linked.bc

intt_red1024b: ntt_red1024_tables.o ntt_red.o 
	$(CC) ${CFLAGS} -c  harness_intt_red1024b.c
	$(GET_BC) harness_intt_red1024b.o
	$(GET_BC) ntt_red1024_tables.o
	$(GET_BC) ntt_red.o
	$(LINKER) --override harness_intt_red1024b.o.bc \
		ntt_red1024_tables.o.bc \
		ntt_red.o.bc -o harness_intt_red1024b.all_linked.bc

ntt_red1024: ntt_red1024_tables.o ntt_red.o 
	$(CC) ${CFLAGS} -c  harness_ntt_red1024.c
	$(GET_BC) harness_ntt_red1024.o
	$(GET_BC) ntt_red1024_tables.o
	$(GET_BC) ntt_red.o
	$(LINKER) --override harness_ntt_red1024.o.bc \
		ntt_red1024_tables.o.bc \
		ntt_red.o.bc -o harness_ntt_red1024.all_linked.bc

ntt_red1024b: ntt_red1024_tables.o ntt_red.o 
	$(CC) ${CFLAGS} -c  harness_ntt_red1024b.c
	$(GET_BC) harness_ntt_red1024b.o
	$(GET_BC) ntt_red1024_tables.o
	$(GET_BC) ntt_red.o
	$(LINKER) --override harness_ntt_red1024b.o.bc \
		ntt_red1024_tables.o.bc \
		ntt_red.o.bc -o harness_ntt_red1024b.all_linked.bc

ntt_red1024c: ntt_red1024_tables.o ntt_red.o 
	$(CC) ${CFLAGS} -c  harness_ntt_red1024c.c
	$(GET_BC) harness_ntt_red1024c.o
	$(GET_BC) ntt_red1024_tables.o
	$(GET_BC) ntt_red.o
	$(LINKER) --override harness_ntt_red1024c.o.bc \
		ntt_red1024_tables.o.bc \
		ntt_red.o.bc -o harness_ntt_red1024c.all_linked.bc


ntt_red1024d: ntt_red1024_tables.o ntt_red.o 
	$(CC) ${CFLAGS} -c  harness_ntt_red1024d.c
	$(GET_BC) harness_ntt_red1024d.o
	$(GET_BC) ntt_red1024_tables.o
	$(GET_BC) ntt_red.o
	$(LINKER) --override harness_ntt_red1024d.o.bc \
		ntt_red1024_tables.o.bc \
		ntt_red.o.bc -o harness_ntt_red1024d.all_linked.bc

ntt_red1024e: ntt_red1024_tables.o ntt_red.o 
	$(CC) ${CFLAGS} -c  harness_ntt_red1024e.c
	$(GET_BC) harness_ntt_red1024e.o
	$(GET_BC) ntt_red1024_tables.o
	$(GET_BC) ntt_red.o
	$(LINKER) --override harness_ntt_red1024e.o.bc \
		ntt_red1024_tables.o.bc \
		ntt_red.o.bc -o harness_ntt_red1024e.all_linked.bc

ntt_red1024f: ntt_red1024_tables.o ntt_red.o 
	$(CC) ${CFLAGS} -c  harness_ntt_red1024f.c
	$(GET_BC) harness_ntt_red1024f.o
	$(GET_BC) ntt_red1024_tables.o
	$(GET_BC) ntt_red.o
	$(LINKER) --override harness_ntt_red1024f.o.bc \
		ntt_red1024_tables.o.bc \
		ntt_red.o.bc -o harness_ntt_red1024f.all_linked.bc

#
# Utility to generate tables
#
make_red_tables: make_red_tables.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o make_red_tables make_red_tables.c

ntt_red1024_tables.h ntt_red1024_tables.c: make_red_tables
	./make_red_tables 1024 1014

%.o: %.c
	$(CC) $(CFLAGS) -c $<

#
# Main modules
#
ntt.o: ntt.c ntt.h

ntt_red.o: ntt_red.c ntt_red.h

ntt_red1024.o: ntt_red1024.c ntt_red.h ntt_red1024.h ntt_red1024_tables.h

#
# Dependencies
#
harness_ntt_tables.o: harness_ntt_tables.c harness_ntt_tables.h

harness_ntt.o: harness_ntt.c ntt.h harness_ntt_tables.h sort.h

harness_ntt_red_tables.o: harness_ntt_tables.c harness_ntt_tables.h

harness_ntt_red.o: harness_ntt_red.c ntt_red.h ntt.h harness_ntt_red_tables.h sort.h

sort.o: sort.c sort.h

harness_ntt1024.o: harness_ntt1024.c ntt.h ntt1024.h ntt1024_tables.h bitrev1024_table.h sort.h 

harness_ntt_red1024.o: harness_ntt_red1024.c ntt.h ntt_red1024.h ntt_red1024_tables.h bitrev1024_table.h sort.h 

install:
	mkdir -p $(INSTALL_DIR)
	$(INSTALL) -m 664 harness_intt_red1024.all_linked.bc $(INSTALL_DIR)
	$(INSTALL) -m 664 harness_intt_red1024b.all_linked.bc $(INSTALL_DIR)
	$(INSTALL) -m 664 harness_ntt_red1024.all_linked.bc $(INSTALL_DIR)
	$(INSTALL) -m 664 harness_ntt_red1024b.all_linked.bc $(INSTALL_DIR)
	$(INSTALL) -m 664 harness_ntt_red1024c.all_linked.bc $(INSTALL_DIR)
	$(INSTALL) -m 664 harness_ntt_red1024d.all_linked.bc $(INSTALL_DIR)
	$(INSTALL) -m 664 harness_ntt_red1024e.all_linked.bc $(INSTALL_DIR)
	$(INSTALL) -m 664 harness_ntt_red1024f.all_linked.bc $(INSTALL_DIR)

#
# Cleanup
#
clean:
	rm -f *~ *.o
	rm -f make_tables make_red_tables make_bitrev_table
	rm -f ntt1024_tables.h ntt1024_tables.c
	rm -f ntt_red1024_tables.h ntt_red1024_tables.c
	rm -f bitrev1024_tables.h bitrev1024_tables.c
	rm -rf *.dSYM

realclean:
	make clean
	rm -Rf bitcode
	rm -Rf .*.bc
	rm -Rf .*.ll
	rm -Rf .*.o
	rm -Rf *.bc
	rm -Rf *.ll
	rm -Rf *.dot

.phony: all clean all_tables
