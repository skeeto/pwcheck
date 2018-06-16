# Database Lookup for "Have I Been Pwned"

This is a little C99 library for efficiently checking passwords against
the [Have I Been Pwned][pwn] dataset. The database is the sorted list of
hashes converted to binary, truncated, and concatenated. Lookups are a
binary search on this memory-mapped file.

Once warmed up, this library can test around 250,000 passwords per
second.

Only POSIX systems are currently supported.

## Compilation

To build, run `make`. It produces:

* `compact`: A command line program for constructing databases.

* `pwcheck`: A simple command line password checking utility.

* `libpwcheck.so`: For use by other programs, particularly those written
  in languages with a foreign functon interface (FFI).

## Database generation

To build a database from the "ordered by hash" dataset, pipe it through
the `compact` command:

    $ ./compact <pwned-passwords-ordered-2.0.txt.xz >pwned.db

Hash truncation is controlled at *compile time* in `config.h`. With the
default configuration, the 2.0 dataset (501m passwords) becomes a 3.8GB
database. Since the database is memory mapped, it is not essential to
have that much physical memory, but it *is* essential for maintaining a
high throughput.

The `pwcheck` convenient utility can query a database without involving
the library. It reads passwords, one per line, on standard input:

    $ echo correcthorsebatterystaple | ./pwcheck pwned.db
    correcthorsebatterystaple: found

    $ echo LyX | ./pwcheck pwned.db
    LyX: not found

## Shared library API

The API for `libpwcheck.so` is very FFI-friendly:

```c
/**
 * Open a database by its filename and return a handle.
 * Returns NULL if the file could not be opened.
 */
struct pwcheck *pwcheck_open(const char *);

/**
 * Close a database and free its resources.
 */
void pwcheck_close(struct pwcheck *);

/**
 * Return 0 if the null-terminated password is not in the database.
 */
int pwcheck_password(const struct pwcheck *, const char *);

/**
 * Return 0 if the given SHA-1 hash is not in the database.
 */
int pwcheck_hash(const struct pwcheck *, const void *);
```

[pwn]: https://haveibeenpwned.com/Passwords
