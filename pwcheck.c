#define _POSIX_C_SOURCE 2
#include "sha1.h"
#include <fcntl.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define DATABASE_TEXT    "pwned-passwords-1.0.txt"
#define DATABASE_BINARY  "pwned-passwords-1.0.bin"
#define RECORD_TEXT      42
#define RECORD_BINARY    20

#define DIE(s) do { perror(s); exit(EXIT_FAILURE); } while (0)

enum db_type {DB_TEXT, DB_BINARY};

struct db {
    void *data;
    size_t count;
    enum db_type type;
};

static void
db_open(struct db *db, const char *dbfile, enum db_type type)
{
    db->type = type;

    int fd = open(dbfile, O_RDONLY);
    if (fd == -1)
        DIE(dbfile);

    struct stat stat[1];
    if (fstat(fd, stat) == -1)
        DIE(dbfile);
    switch (type) {
        case DB_TEXT:
            db->count = stat->st_size / RECORD_TEXT;
            break;
        case DB_BINARY:
            db->count = stat->st_size / RECORD_BINARY;
            break;
    }

    db->data = mmap(0, stat->st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (db->data == MAP_FAILED)
        DIE(dbfile);
    close(fd);
}

static void
db_close(struct db *db)
{
    switch (db->type) {
        case DB_TEXT:
            munmap(db->data, db->count * RECORD_TEXT);
            break;
        case DB_BINARY:
            munmap(db->data, db->count * RECORD_BINARY);
            break;
    }
}

static int
txtcmp(const void *a, const void *b)
{
    return memcmp(a, b, SHA1_DIGEST_SIZE * 2);
}

static int
bincmp(const void *a, const void *b)
{
    return memcmp(a, b, SHA1_DIGEST_SIZE);
}

static char *
db_search(struct db *db, char *password)
{
    unsigned char buf[SHA1_DIGEST_SIZE];
    char txt[SHA1_DIGEST_SIZE * 2];

    SHA1_CTX ctx[1];
    SHA1_Init(ctx);
    SHA1_Update(ctx, (void *)password, strlen(password));
    SHA1_Final(ctx, buf);
    switch (db->type) {
        case DB_TEXT:
            for (int i = 0; i < SHA1_DIGEST_SIZE; i++) {
                txt[i * 2 + 0] = "0123456789ABCDEF"[buf[i] >> 4];
                txt[i * 2 + 1] = "0123456789ABCDEF"[buf[i] & 0xf];
            }
            return bsearch(txt, db->data, db->count, RECORD_TEXT, txtcmp);
        case DB_BINARY:
            return bsearch(buf, db->data, db->count, RECORD_BINARY, bincmp);
    }
    abort();
}

int
main(int argc, char **argv)
{
    enum db_type type = DB_TEXT;
    const char *dbfile = 0;

    int option;
    while ((option = getopt(argc, argv, "bd:t")) != -1) {
        switch (option) {
            case 'b':
                type = DB_BINARY;
                break;
            case 'd':
                dbfile = optarg;
                break;
            case 't':
                type = DB_TEXT;
                break;
        }
    }

    /* Determine default database name from type */
    if (!dbfile) {
        switch (type) {
            case DB_TEXT:
                dbfile = DATABASE_TEXT;
                break;
            case DB_BINARY:
                dbfile = DATABASE_BINARY;
                break;
        }
    }

    /* Look up each word from standard input */
    struct db db;
    db_open(&db, dbfile, type);
    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        size_t last = strlen(line) - 1;
        if (line[last] == '\n')
            line[last] = 0;
        char *result = db_search(&db, line);
        printf("%s: %s\n", line, result ? "found" : "not found");
    }
    db_close(&db);
}
