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
#include <unistd.h>
#include "map.h"


static const char *cmd;  // name of the program, usually argv[0]


const char *error(void)
{
    assert(errno != 0);
    return strerror(errno);
}


void *xmalloc(size_t size)
{
    void *ret = malloc(size);

    if (ret == NULL) {
	fprintf(stderr, "%s: Memory allocation failed: %s\n", cmd, error());
	exit(EXIT_FAILURE);
    }

    return ret;
}


void *xcalloc(size_t nmemb, size_t size)
{
    void *ret = calloc(nmemb, size);

    if (ret == NULL) {
	fprintf(stderr, "%s: Memory allocation failed: %s\n", cmd, error());
	exit(EXIT_FAILURE);
    }

    return ret;
}


static bool is_regular_dir(const char *path)
{
    struct stat st;

    if (stat(path, &st) == -1) {
	return false;
    }

    return S_ISDIR(st.st_mode) && ! S_ISLNK(st.st_mode);
}


static void add_dir(int fd, const struct map *watching, const char *path, bool initial)
{
    // Add this directory

    int watchfd;
    uint32_t mask = IN_CREATE | IN_DELETE | IN_ONLYDIR;

    if ((watchfd = inotify_add_watch(fd, path, mask)) == -1) {
	if (initial || errno != ENOTDIR) {
	    fprintf(stderr, "%s: Adding %s failed: %s\n", cmd, path, error());
	    exit(EXIT_FAILURE);
	}
	
	return;  // errno == ENOTDIR
    }

    printf("Added %s\n", path);

    // Add all subdirs

    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(path)) == NULL) {
	fprintf(stderr, "%s: Cannot open %s: %s\n", cmd, path, error());
	exit(EXIT_FAILURE);
    }

    while (errno = 0, (ent = readdir(dir)) != NULL) {
	if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
	    continue;
	}

	size_t len = strlen(path) + 1 + strlen(ent->d_name) + 1;
	char *child_path = xmalloc(len);
	snprintf(child_path, len, "%s/%s", path, ent->d_name);

	if (is_regular_dir(child_path)) {
	    add_dir(fd, watching, child_path, false);
	}

	free(child_path);
    }

    closedir(dir);

    if (errno != 0) {
	fprintf(stderr, "%s: Adding %s entry failed: %s\n", cmd, path, error());
    }
}


int main(int argc, char **argv)
{
    int fd;
    struct map *watching;

    // Set cmd so we can have pretty error messages that start with
    // the program name

    {
	if (argc > 0) {
	    cmd = argv[0];
	} else {
	    cmd = "wde";
	}
    }

    // Print usage if no arguments were supplied

    if (argc <= 1) {
	fprintf(stderr, "%s: Missing directories\n", cmd);
	return EXIT_FAILURE;
    }

    // Initialize inotify

    if ((fd = inotify_init()) == -1) {
	fprintf(stderr, "%s: Init failed: %s\n", cmd, error());
	return EXIT_FAILURE;
    }

    // Initialize lookup map

    if ((watching = map_init()) == NULL) {
	fprintf(stderr, "%s: Init failed: %s\n", cmd, error());
	return EXIT_FAILURE;
    }

    // Each user argument should be a directory that is to be watched

    for (int i = 1; i < argc; ++i) {
	add_dir(fd, watching, argv[i], true);
    }

    return EXIT_SUCCESS;
}
