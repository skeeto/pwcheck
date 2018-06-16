CC     = c99
CFLAGS = -Wall -Wextra -O3

all: pwcheck libpwcheck.so compact

pwcheck: pwcheck.c sha1.c sha1.h
	$(CC) $(LDFLAGS) $(CFLAGS) -DCMDLINE -o $@ pwcheck.c sha1.c $(LDLIBS)

libpwcheck.so: pwcheck.c sha1.c sha1.h
	$(CC) $(LDFLAGS) -shared $(CFLAGS) -fPIC -fvisibility=hidden \
		-o $@ pwcheck.c sha1.c $(LDLIBS)

compact: compact.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ compact.c $(LDLIBS)

clean:
	rm -f pwcheck compact libpwcheck.so
