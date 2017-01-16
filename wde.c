#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include "map.h"


static const char *cmd;  // name of the program, usually argv[0]


/*
 * Helper functions
 */


static const char *error(void)
{
    assert(errno != 0);
    return strerror(errno);
}


static void *xmalloc(size_t size)
{
    void *ret = malloc(size);

    if (ret == NULL) {
	fprintf(stderr, "%s: Memory allocation failed: %s\n", cmd, error());
	exit(EXIT_FAILURE);
    }

    return ret;
}


/*
 * Init logic
 */


static void add_dir(int fd, struct map *watching, const char *path, bool initial)
{
    int watchfd;
    uint32_t mask = IN_CREATE | IN_DELETE | IN_ONLYDIR;

    if ((watchfd = inotify_add_watch(fd, path, mask)) == -1) {
	fprintf(stderr, "%s: Adding %s failed: %s\n", cmd, path, error());
	exit(EXIT_FAILURE);
    }

    if (map_insert(watching, watchfd, path) == -1) {
	fprintf(stderr, "%s: Broken table: %s\n", cmd, error());
	exit(EXIT_FAILURE);
    }
}


static void init(int argc, char **argv, int *fd, struct map **watching)
{
    // Set cmd so we can have pretty error messages that start with
    // the program name

    if (argc > 0) {
	cmd = argv[0];
    } else {
	cmd = "wde";
    }

    // Print usage if no arguments were supplied

    if (argc <= 1) {
	fprintf(stderr, "%s: Missing directories\n", cmd);
	exit(EXIT_FAILURE);
    }

    // Initialize inotify

    if ((*fd = inotify_init()) == -1) {
	fprintf(stderr, "%s: Init failed: %s\n", cmd, error());
	exit(EXIT_FAILURE);
    }

    // Initialize lookup map

    if ((*watching = map_init()) == NULL) {
	fprintf(stderr, "%s: Init failed: %s\n", cmd, error());
	exit(EXIT_FAILURE);
    }

    // Each user argument should be a directory that is to be watched

    for (int i = 1; i < argc; ++i) {
	add_dir(*fd, *watching, argv[i], true);
    }
}

/*
 * Watch logic
 */


static void print_event(const struct inotify_event *event, const struct map *paths)
{
    const char *dir;

    if ((dir = map_get(paths, event->wd)) == NULL) {
	fprintf(stderr, "%s: Cannot find path for wd %d\n", cmd, event->wd);
	exit(EXIT_FAILURE);
    }

    size_t len = strlen(dir) + 1 + strlen(event->name) + 1;
    char *full_path = xmalloc(len);
    snprintf(full_path, len, "%s/%s", dir, event->name);

    if (event->mask & IN_CREATE) {
	fputs("-> ", stdout);
    } else if (event->mask & IN_DELETE) {
	fputs("<- ", stdout);
    } else {
	fprintf(stderr, "%s: Unexpected event mask 0x%x\n", cmd, event->mask);
	exit(EXIT_FAILURE);
    }

    puts(full_path);
    free(full_path);
}


static void read_and_print(int fd, struct map *watching)
{
    // select(2) blocks until fd is ready for reading

    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd, &set);

    if (select(fd + 1, &set, NULL, NULL, NULL) == -1) {
	fprintf(stderr, "%s: select(2) failed: %s", cmd, strerror(errno));
	exit(EXIT_FAILURE);
    }

    // Find out how many bytes we may read. We assume that Linux never
    // gives us less than the full struct to read.

    unsigned long bytes_ready = 0;
    if (ioctl(fd, FIONREAD, &bytes_ready) == -1) {
	fprintf(stderr, "%s: ioctl(2) failed: %s", cmd, strerror(errno));
	exit(EXIT_FAILURE);
    }

    // Read from fd

    uint8_t *buffer = xmalloc(bytes_ready);
    ssize_t rres = read(fd, buffer, bytes_ready);

    if (rres == -1) {
	fprintf(stderr, "%s: Cannot read(2) ionotify fd: %s\n", cmd, strerror(errno));
	exit(EXIT_FAILURE);
    } else if ((size_t) rres != bytes_ready) {
	fprintf(stderr, "%s: Expected %lu bytes, got %ld", cmd, bytes_ready, rres);
	exit(EXIT_FAILURE);
    }

    // The read bytes can contain more than just one event

    uint8_t *bufpos = buffer;

    while (bufpos < buffer + bytes_ready) {
	struct inotify_event *event = (struct inotify_event *) bufpos;
	print_event(event, watching);
	bufpos = bufpos + sizeof(*event) + event->len;
    }

    free(buffer);
}


/*
 * Entry point
 */


int main(int argc, char **argv)
{
    int fd;
    struct map *watching;

    init(argc, argv, &fd, &watching);

    while (true) {
	read_and_print(fd, watching);
    }

    return EXIT_SUCCESS;  // never reached
}
