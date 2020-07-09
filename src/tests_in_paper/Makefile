#
# all tests
#
tests=test_intt_red1024 test_intt_red1024b \
  test_ntt_red1024 test_ntt_red1024b test_ntt_red1024c \
  test_ntt_red1024d test_ntt_red1024e test_ntt_red1024f

CC?=clang
CFLAGS=-Wall -I../

#
# We assume ../intervals.o ../red_bounds.o ../ntt_red_interval.o ../ntt_red1024_tables.o all exist
# and are up to date.
#
obj=../intervals.o ../red_bounds.o ../ntt_red_interval.o ../ntt_red1024_tables.o

all: $(tests)

test_intt_red1024: test_intt_red1024.c
	$(CC) $(CFLAGS) -o $@ $^ $(obj)

test_intt_red1024b: test_intt_red1024b.c
	$(CC) $(CFLAGS) -o $@ $^ $(obj)

test_ntt_red1024: test_ntt_red1024.c
	$(CC) $(CFLAGS) -o $@ $^ $(obj)

test_ntt_red1024b: test_ntt_red1024b.c
	$(CC) $(CFLAGS) -o $@ $^ $(obj)

test_ntt_red1024c: test_ntt_red1024c.c
	$(CC) $(CFLAGS) -o $@ $^ $(obj)

test_ntt_red1024d: test_ntt_red1024d.c
	$(CC) $(CFLAGS) -o $@ $^ $(obj)

test_ntt_red1024e: test_ntt_red1024e.c
	$(CC) $(CFLAGS) -o $@ $^ $(obj)

test_ntt_red1024f: test_ntt_red1024f.c
	$(CC) $(CFLAGS) -o $@ $^ $(obj)

#
# Clean up
#
clean:
	rm -f $(tests)
	rm -f *.o
