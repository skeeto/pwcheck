CC     = c99
CFLAGS = -Wall -Wextra -O3

all: pwcheck compact

pwcheck: pwcheck.c sha1.c sha1.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ pwcheck.c sha1.c $(LDLIBS)

compact: compact.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ compact.c $(LDLIBS)

clean:
	rm -f pwcheck compact
