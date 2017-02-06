#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "map.h"


static const char *cmd = "wde";  // name of the program, usually argv[0]


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


static bool a_before_b(const struct timespec *a, const struct timespec *b)
{
    if (a->tv_sec == b->tv_sec) {
	return a->tv_nsec < b->tv_nsec;
    } else {
	return a->tv_sec < b->tv_sec;
    }
}


/*
 * Returns true if the directory at path was modified before cutoff.
 */
static bool is_dir_older_than(const char *path, const struct timespec *t)
{
    struct stat ss;

    if ((lstat(path, &ss)) == -1) {
	fprintf(stderr, "%s: %s: %s\n", cmd, path, error());
	return false;
    }

    return S_ISDIR(ss.st_mode) && a_before_b(&ss.st_mtim, t);
}


static int add_dir(int fd, struct map *watching, const char *path,
		   struct timespec *cutoff, bool recursive)
{
    // Add the current directory

    int watchfd;
    uint32_t mask = IN_CREATE | IN_DELETE | IN_ONLYDIR;

    if ((watchfd = inotify_add_watch(fd, path, mask)) == -1) {
	fprintf(stderr, "%s: Adding %s failed: %s\n", cmd, path, error());
	return -1;
    }

    if (map_insert(watching, watchfd, path) == -1) {
	fprintf(stderr, "%s: Broken table: %s\n", cmd, error());
	return -1;
    }

    if (! recursive) {
	return 0;
    }

    // Scan the directory for sub-directories

    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(path)) == NULL) {
	return -1;
    }

    while (errno = 0, (ent = readdir(dir)) != NULL) {
	if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
	    continue;
	}

	size_t len = strlen(path) + 1 + strlen(ent->d_name) + 1;
	char *full_path = xmalloc(len);
	snprintf(full_path, len, "%s/%s", path, ent->d_name);
	if (is_dir_older_than(full_path, cutoff)) {  // racy, but what else can you do?
	    add_dir(fd, watching, full_path, cutoff, true);
	}
	free(full_path);
    }

    int ret = errno == 0 ? 0 : - 1;
    closedir(dir);
    return ret;
}


static void init(int argc, char **argv, int *fd, struct map **watching)
{
    // Set cmd so we can have pretty error messages that start with
    // the program name

    if (argc >= 1) {
	cmd = argv[0];
    }

    // Parse arguments

    bool recursive = false;

    int c;
    while ((c = getopt(argc, argv, "r")) != -1) {
	switch (c) {
	case 'r':
	    recursive = true;
	    break;

	default:
	    exit(EXIT_FAILURE);  // getopt shows error message
	}
    }

    // Print usage if no directories were supplied

    if (optind == argc) {
	fprintf(stderr, "usage: %s [-r] [DIRECTORY]...\n", cmd);
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

    struct timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now) == -1) {
	fprintf(stderr, "%s: cannot get current time: %s\n", cmd, error());
	exit(EXIT_FAILURE);
    }

    for (int i = optind; i < argc; ++i) {
	if (add_dir(*fd, *watching, argv[i], &now, recursive) == -1) {
	    exit(EXIT_FAILURE);
	}
    }
}

/*
 * Watch logic
 */


static void handle_event(const struct inotify_event *event, struct map *paths)
{
    const char *dir;

    if ((dir = map_get(paths, event->wd)) == NULL) {
	fprintf(stderr, "%s: Cannot find path for wd %d\n", cmd, event->wd);
	exit(EXIT_FAILURE);
    }

    if (event->mask & (IN_CREATE | IN_DELETE)) {
	size_t len = strlen(dir) + 1 + strlen(event->name) + 1;
	char *full_path = xmalloc(len);
	snprintf(full_path, len, "%s/%s", dir, event->name);

	if (event->mask & IN_CREATE) {
	    printf("-> %s\n", full_path);
	} else if (event->mask & IN_DELETE) {
	    printf("<- %s\n", full_path);
	}

	free(full_path);
    } else if (event->mask & (IN_DELETE_SELF | IN_IGNORED)) {
	printf("<- %s\n", dir);
	// We may reach this "else if" body multiple times for the
	// same file, which is why we do not check for errors of
	// map_remove here.
	map_remove(paths, event->wd);
    } else {
	fprintf(stderr, "%s: Unexpected event mask 0x%x\n", cmd, event->mask);
	exit(EXIT_FAILURE);
    }
}


static size_t read_and_print(int fd, struct map *watching)
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
    size_t handled = 0;

    while (bufpos < buffer + bytes_ready) {
	struct inotify_event *event = (struct inotify_event *) bufpos;
	handle_event(event, watching);
	bufpos = bufpos + sizeof(*event) + event->len;

	handled += 1;
    }

    free(buffer);
    return handled;
}


/*
 * Entry point
 */


int main(int argc, char **argv)
{
    int fd;
    struct map *watching;

    init(argc, argv, &fd, &watching);

    while (map_items(watching) > 0) {
	read_and_print(fd, watching);
    }

    close(fd);
    map_destroy(watching);

    return EXIT_SUCCESS;
}
