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

#define DIE(s) do { perror(s); exit(EXIT_FAILURE); } while (0)

struct db {
    void *data;
    size_t count;
};

static void
db_open(struct db *db, const char *dbfile)
{
    int fd = open(dbfile, O_RDONLY);
    if (fd == -1)
        DIE(dbfile);

    struct stat stat[1];
    if (fstat(fd, stat) == -1)
        DIE(dbfile);
    db->count = stat->st_size / HASH_LENGTH;

    db->data = mmap(0, stat->st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (db->data == MAP_FAILED)
        DIE(dbfile);
    close(fd);
}

static void
db_close(struct db *db)
{
    munmap(db->data, db->count * HASH_LENGTH);
}

static int
bincmp(const void *a, const void *b)
{
    return memcmp(a, b, HASH_LENGTH);
}

static char *
db_search(struct db *db, char *password)
{
    unsigned char buf[SHA1_DIGEST_SIZE];

    SHA1_CTX ctx[1];
    SHA1_Init(ctx);
    SHA1_Update(ctx, (void *)password, strlen(password));
    SHA1_Final(ctx, buf);
    return bsearch(buf, db->data, db->count, HASH_LENGTH, bincmp);
}

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

    /* Look up each word from standard input */
    struct db db;
    db_open(&db, dbfile);
    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        size_t last = strlen(line) - 1;
        if (line[last] == '\n')
            line[last] = 0;
        char *result = db_search(&db, line);
        printf("%s: %s\n", line, result ? "found" : "not found");
    }
#ifndef __CYGWIN__
    db_close(&db);
#else
    (void)db_close;
#endif
}
