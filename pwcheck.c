#define _POSIX_C_SOURCE 2
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "sha1.h"
#include "config.h"
#include "pwcheck.h"

struct pwcheck {
    void *data;
    size_t count;
};

static int
db_open(struct pwcheck *db, const char *dbfile)
{
    int fd = open(dbfile, O_RDONLY);
    if (fd == -1)
        return 0;

    struct stat stat[1];
    if (fstat(fd, stat) == -1) {
        close(fd);
        return 0;
    }
    db->count = stat->st_size / HASH_LENGTH;

    db->data = mmap(0, stat->st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return db->data != MAP_FAILED;
}

static void
db_close(struct pwcheck *db)
{
    munmap(db->data, db->count * HASH_LENGTH);
}

static int
bincmp(const void *a, const void *b)
{
    return memcmp(a, b, HASH_LENGTH);
}

static char *
db_search(const struct pwcheck *db, const void *hash)
{
    return bsearch(hash, db->data, db->count, HASH_LENGTH, bincmp);
}

static void
password_hash(unsigned char buf[SHA1_DIGEST_SIZE], const char *password)
{
    SHA1_CTX ctx[1];
    SHA1_Init(ctx);
    SHA1_Update(ctx, (void *)password, strlen(password));
    SHA1_Final(ctx, buf);
}

#ifndef CMDLINE

/* Shared library interface */

PWCHECK_API
struct pwcheck *
pwcheck_open(const char *filename)
{
    struct pwcheck *db = malloc(sizeof(*db));
    if (!db)
        return 0;
    if (!db_open(db, filename)) {
        free(db);
        return 0;
    }
    return db;
}

PWCHECK_API
void
pwcheck_close(struct pwcheck *db)
{
    db_close(db);
    free(db);
}

PWCHECK_API
int
pwcheck_password(const struct pwcheck *db, const char *password)
{
    unsigned char buf[SHA1_DIGEST_SIZE];
    password_hash(buf, password);
    return !!db_search(db, buf);
}

PWCHECK_API
int
pwcheck_hash(const struct pwcheck *db, const void *hash)
{
    return !!db_search(db, hash);
}

#else

/* Command line interface */

static void
usage(FILE *f)
{
    fprintf(f, "usage: pwcheck DBFILE\n");
}

int
main(int argc, char **argv)
{
    char *dbfile = 0;

    int option;
    while ((option = getopt(argc, argv, "h")) != -1) {
        switch (option) {
            case 'h':
                usage(stdout);
                exit(EXIT_SUCCESS);
            default:
                exit(EXIT_FAILURE);
        }
    }

    dbfile = argv[optind];
    if (!dbfile) {
        fprintf(stderr, "pwcheck: requires a database file\n");
        usage(stderr);
        exit(EXIT_FAILURE);
    }

    struct pwcheck db;
    if (!db_open(&db, dbfile)) {
        perror(dbfile);
        exit(EXIT_FAILURE);
    }

    /* Look up each word from standard input */
    char line[256];
    unsigned char buf[SHA1_DIGEST_SIZE];
    while (fgets(line, sizeof(line), stdin)) {
        size_t last = strlen(line) - 1;
        if (line[last] == '\n')
            line[last] = 0;
        password_hash(buf, line);
        char *result = db_search(&db, buf);
        printf("%s: %s\n", line, result ? "found" : "not found");
    }

#ifdef __CYGWIN__
    db_close(&db);
#else
    (void)db_close; // munmap() is ridiculously slow on Cygwin
#endif /* __CYGWIN__ */
}

#endif /* CMDLINE */
