#ifndef PWCHECK_H
#define PWCHECK_H

#if defined(__GNUC__) || defined(__clang__)
#  define PWCHECK_API __attribute__((visibility("default")))
#else
#  define PWCHECK_API
#endif

struct pwcheck;

/**
 * Open a database by its filename and return a handle.
 * Returns NULL if the file could not be opened.
 */
PWCHECK_API
struct pwcheck *pwcheck_open(const char *);

/**
 * Close a database and free its resources.
 */
PWCHECK_API
void pwcheck_close(struct pwcheck *);

/**
 * Return 0 if the null-terminated password is not in the database.
 */
PWCHECK_API
int pwcheck_password(const struct pwcheck *, const char *);

/**
 * Return 0 if the given SHA1 hash is not in the database.
 */
PWCHECK_API
int pwcheck_hash(const struct pwcheck *, const void *);

#endif
