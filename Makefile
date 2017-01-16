#
# -D_BSD_SOURCE is needed on Linux otherwise random()
# is not declared in <stdlib.h>
#

CC=clang
CPPFLAGS=-DNDEBUG -D_BSD_SOURCE
CFLAGS=-Wall -std=c99 -O3
# CFLAGS=-Wall -std=c99 -g

all: test_ntt test_ntt16 test_ntt256 test_ntt512 test_ntt1024 \
	kat_mul1024 speed_mul1024 kat_mul1024_red speed_mul1024_red \
	test_ntt_red16 test_ntt_red256 test_ntt_red512 test_ntt_red1024 \
	test_ntt_red test_red_bounds

#
# Utility to generate tables
#
make_tables: make_tables.c
	$(CC) -Wall -g -o make_tables make_tables.c

make_red_tables: make_red_tables.c
	$(CC) -Wall -g -o make_red_tables make_red_tables.c

make_bitrev_table: make_bitrev_table.c
	$(CC) -Wall -g -o make_bitrev_table make_bitrev_table.c

#
# Auto-generated source files
#
# 'make_tables <size> <psi>' generates 
# ntt<size>_tables.h and ntt<size>_tables.c
#
# 'make_red_tables <size> <psi>' generates 
# ntt_red<size>_tables.h and ntt_red<size>_tables.c
#
# 'make_bitrev_table <size>' generates 
# bitrev<size>_table.h and bitrev<size>_table.c
#
ntt16_tables.h ntt16_tables.c: make_tables
	./make_tables 16 1212

ntt256_tables.h ntt256_tables.c: make_tables
	./make_tables 256 1002

ntt512_tables.h ntt512_tables.c: make_tables
	./make_tables 512 1003

ntt1024_tables.h ntt1024_tables.c: make_tables
	./make_tables 1024 1014

ntt_red16_tables.h ntt_red16_tables.c: make_red_tables
	./make_red_tables 16 1212

ntt_red256_tables.h ntt_red256_tables.c: make_red_tables
	./make_red_tables 256 1002

ntt_red512_tables.h ntt_red512_tables.c: make_red_tables
	./make_red_tables 512 1003

ntt_red1024_tables.h ntt_red1024_tables.c: make_red_tables
	./make_red_tables 1024 1014

bitrev16_table.h bitrev16_table.c: make_bitrev_table
	./make_bitrev_table 16

bitrev256_table.h bitrev256_table.c: make_bitrev_table
	./make_bitrev_table 256

bitrev512_table.h bitrev512_table.c: make_bitrev_table
	./make_bitrev_table 512

bitrev1024_table.h bitrev1024_table.c: make_bitrev_table
	./make_bitrev_table 1024

all_tables: ntt16_tables.h ntt16_tables.c ntt256_tables.h ntt256_tables.c \
	ntt512_tables.h ntt512_tables.c ntt1024_tables.h ntt1024_tables.c \
	ntt_red16_tables.h ntt_red16_tables.c ntt_red256_tables.h ntt_red256_tables.c \
	ntt_red512_tables.h ntt_red512_tables.c ntt_red1024_tables.h ntt_red1024_tables.c
	bitrev16_tables.h bitrev16_tables.c bitrev256_tables.h bitrev256_tables.c \
	bitrev512_tables.h bitrev512_tables.c bitrev1024_tables.h bitrev1024_tables.c

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<


#
# Main modules
#
ntt.o: ntt.c ntt.h

ntt_red.o: ntt_red.c ntt_red.h

red_bounds.o: red_bounds.c red_bounds.h

ntt16.o: ntt16.c ntt.h ntt16.h ntt16_tables.h

ntt256.o: ntt256.c ntt.h ntt256.h ntt256_tables.h

ntt512.o: ntt512.c ntt.h ntt512.h ntt512_tables.h

ntt1024.o: ntt1024.c ntt.h ntt1024.h ntt1024_tables.h

ntt_red16.o: ntt_red16.c ntt_red.h ntt_red16.h ntt_red16_tables.h

ntt_red256.o: ntt_red256.c ntt_red.h ntt_red256.h ntt_red256_tables.h

ntt_red512.o: ntt_red512.c ntt_red.h ntt_red512.h ntt_red512_tables.h

ntt_red1024.o: ntt_red1024.c ntt_red.h ntt_red1024.h ntt_red1024_tables.h



#
# Test code
#
test_ntt: test_ntt.o ntt.o test_ntt_tables.o test_bitrev_tables.o sort.o
	$(CC) $^ -o $@

test_ntt_red: test_ntt_red.o ntt_red.o ntt.o test_ntt_red_tables.o \
	  test_bitrev_tables.o sort.o
	$(CC) $^ -o $@

test_ntt16: test_ntt16.o ntt16.o ntt16_tables.o bitrev16_table.o ntt.o sort.o
	$(CC) $^ -o $@

test_ntt256: test_ntt256.o ntt256.o ntt256_tables.o bitrev256_table.o ntt.o sort.o
	$(CC) $^ -o $@

test_ntt512: test_ntt512.o ntt512.o ntt512_tables.o bitrev512_table.o ntt.o sort.o
	$(CC) $^ -o $@

test_ntt1024: test_ntt1024.o ntt1024.o ntt1024_tables.o bitrev1024_table.o ntt.o sort.o
	$(CC) $^ -o $@

test_ntt_red16: test_ntt_red16.o ntt_red16.o ntt_red16_tables.o bitrev16_table.o ntt.o \
	  ntt_red.o sort.o
	$(CC) $^ -o $@

test_ntt_red256: test_ntt_red256.o ntt_red256.o ntt_red256_tables.o bitrev256_table.o ntt.o \
	  ntt_red.o sort.o
	$(CC) $^ -o $@

test_ntt_red512: test_ntt_red512.o ntt_red512.o ntt_red512_tables.o bitrev512_table.o ntt.o \
	  ntt_red.o sort.o
	$(CC) $^ -o $@

test_ntt_red1024: test_ntt_red1024.o ntt_red1024.o ntt_red1024_tables.o bitrev1024_table.o \
	  ntt.o ntt_red.o sort.o
	$(CC) $^ -o $@


speed_mul1024: speed_mul1024.o ntt1024.o ntt1024_tables.o ntt.o sort.o
	$(CC) $^ -o $@

kat_mul1024: kat_mul1024.o ntt1024.o ntt1024_tables.o ntt.o data_poly1024.o
	$(CC) $^ -o $@

kat_mul1024_red: kat_mul1024_red.o ntt_red1024.o ntt_red1024_tables.o ntt_red.o data_poly1024.o
	$(CC) $^ -o $@

speed_mul1024_red: speed_mul1024_red.o ntt_red1024.o ntt_red1024_tables.o ntt_red.o sort.o
	$(CC) $^ -o $@

test_red_bounds: test_red_bounds.o red_bounds.o test_ntt_red_tables.o
	$(CC) $^ -o $@


#
# Dependencies
#
test_ntt_tables.o: test_ntt_tables.c test_ntt_tables.h

test_ntt.o: test_ntt.c ntt.h test_ntt_tables.h sort.h

test_ntt_red_tables.o: test_ntt_tables.c test_ntt_tables.h

test_ntt_red.o: test_ntt_red.c ntt_red.h ntt.h test_ntt_red_tables.h sort.h

sort.o: sort.c sort.h

test_ntt16.o: test_ntt16.c ntt.h ntt16.h ntt16_tables.h bitrev16_table.h sort.h 

test_ntt256.o: test_ntt256.c ntt.h ntt256.h ntt256_tables.h bitrev256_table.h sort.h 

test_ntt512.o: test_ntt512.c ntt.h ntt512.h ntt512_tables.h bitrev512_table.h sort.h 

test_ntt1024.o: test_ntt1024.c ntt.h ntt1024.h ntt1024_tables.h bitrev1024_table.h sort.h 

test_ntt_red16.o: test_ntt_red16.c ntt.h ntt_red16.h ntt_red16_tables.h bitrev16_table.h sort.h 

test_ntt_red256.o: test_ntt_red256.c ntt.h ntt_red256.h ntt_red256_tables.h bitrev256_table.h sort.h 

test_ntt_red512.o: test_ntt_red512.c ntt.h ntt_red512.h ntt_red512_tables.h bitrev512_table.h sort.h 

test_ntt_red1024.o: test_ntt_red1024.c ntt.h ntt_red1024.h ntt_red1024_tables.h bitrev1024_table.h sort.h 

speed_mul1024.o: speed_mul1024.c ntt.h ntt1024.h ntt1024_tables.h sort.h

speed_mul1024_red.o: speed_mul1024_red.c ntt_red.h ntt_red1024.h ntt_red1024_tables.h sort.h

kat_mul1024.o: kat_mul1024.c ntt.h ntt1024.h ntt1024_tables.h data_poly1024.h

kat_mul1024_red.o: kat_mul1024_red.c ntt_red.h ntt_red1024.h ntt_red1024_tables.h data_poly1024.h

data_poly1024.o: data_poly1024.c data_poly1024.h

test_red_bounds.o: test_red_bounds.c red_bounds.h test_ntt_red_tables.h


#
# Cleanup
#
clean:
	rm -f *~ *.o
	rm -f test_ntt test_ntt_red test_red test_mod test_shift \
	  test_ntt16 test_ntt256 test_ntt512 test_ntt1024 \
	  test_ntt_red16 test_ntt_red256 test_ntt_red512 test_ntt_red1024 \
	  make_tables make_red_tables make_bitrev_table \
          kat_mul1024 speed_mul1024 kat_mul1024_red speed_mul1024_red test_red_bounds
	rm -f ntt16_tables.h ntt16_tables.c
	rm -f ntt256_tables.h ntt256_tables.c
	rm -f ntt512_tables.h ntt512_tables.c
	rm -f ntt1024_tables.h ntt1024_tables.c
	rm -f ntt_red16_tables.h ntt_red16_tables.c
	rm -f ntt_red256_tables.h ntt_red256_tables.c
	rm -f ntt_red512_tables.h ntt_red512_tables.c
	rm -f ntt_red1024_tables.h ntt_red1024_tables.c
	rm -f bitrev16_tables.h bitrev16_tables.c
	rm -f bitrev256_tables.h bitrev256_tables.c
	rm -f bitrev512_tables.h bitrev512_tables.c
	rm -f bitrev1024_tables.h bitrev1024_tables.c
	rm -rf *.dSYM

.phony: all clean all_tables
