#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <map>
#include <string>


static const char *cmd;  // Name of the program, i.e. argv[0] if available


/*
 * Start watching a single directory.
 */
static int add_directory(int &fd, std::map<int, const char *> &watching, const char *path, bool initial=true)
{
    // Add path if it is a directory

    int watchfd;
    uint32_t mask = IN_CREATE | IN_DELETE | IN_ONLYDIR;

    if ((watchfd = inotify_add_watch(fd, path, mask)) == -1) {
	if (initial || errno != ENOTDIR) {
	    fprintf(stderr, "%s: Adding %s failed: %s\n", cmd, path, strerror(errno));
	    return EXIT_FAILURE;
	}
    }

    watching.insert(std::make_pair(watchfd, path));

    // Add all subdirs

    struct dirent *entr;


    return EXIT_SUCCESS;
}


/*
 * Init the program, i.e. init inotify and crawl for directories.
 * Returns EXIT_SUCCESS on success.
 */
static int init(int argc, char **argv, int &fd, std::map<int, const char *> &watching)
{
    // argv[0]

    if (argc > 0) {
	cmd = argv[0];
    } else {
	cmd = "wde";
    }

    // Syntax

    if (argc <= 1) {
	fprintf(stderr, "%s: Missing arguments\n", cmd);
	return EXIT_FAILURE;
    }


    // Initialize inotify

    if ((fd = inotify_init()) == -1) {
	fprintf(stderr, "%s: Init failed: %s\n", cmd, strerror(errno));
	return EXIT_FAILURE;
    }

    // Add all directories

    for (int i = 1; i < argc; ++i) {
	if (add_directory(fd, watching, argv[i]) != EXIT_SUCCESS) {
	    return EXIT_FAILURE;
	}
    }

    return EXIT_SUCCESS;
}

/*
 * Wait for notifications and print them on stdout.
 * Returns after a single read(2) on fd. Return value EXIT_SUCCESS indicates success.
 */
static int print_changes(int fd, const std::map<int, const char *> watching)
{
    // select(2) blocks until fd is ready for reading

    {
	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);

	if (select(fd + 1, &set, NULL, NULL, NULL) == -1) {
	    fprintf(stderr, "%s: select(2) failed: %s", cmd, strerror(errno));
	    return EXIT_FAILURE;
	}
    }

    // Find out how many bytes we may read. We assume that Linux never
    // gives us less than the full struct to read.

    unsigned long bytes_ready;

    if (ioctl(fd, FIONREAD, &bytes_ready) == -1) {
	fprintf(stderr, "%s: ioctl(2) failed: %s", cmd, strerror(errno));
	return EXIT_FAILURE;
    }

    // Read from fd and print out the changes

    {
	ssize_t rres;
	uint8_t *buffer = new uint8_t[bytes_ready];

	rres = read(fd, buffer, bytes_ready);

	if (rres == -1) {
	    fprintf(stderr, "%s: Cannot read(2) ionotify fd: %s\n", cmd, strerror(errno));
	    return EXIT_FAILURE;
	} else if ((size_t) rres != bytes_ready) {
	    fprintf(stderr, "%s: Expected %lu bytes, got %ld", cmd, bytes_ready, rres);
	    return EXIT_FAILURE;
	}

	uint8_t *bufpos = buffer;

	while (bufpos < buffer + bytes_ready) {
	    struct inotify_event *event = (struct inotify_event *) bufpos;

	    if (event->mask & IN_CREATE) {
		printf("-> %s\n", event->name);
	    } else if (event->mask & IN_DELETE) {
		printf("<- %s\n", event->name);
	    } else {
		fprintf(stderr, "%s: Unexpected event mask 0x%x\n", cmd, event->mask);
		return EXIT_FAILURE;
	    }

	    bufpos = bufpos + sizeof(*event) + event->len;
	}

	delete buffer;
    }

    return EXIT_SUCCESS;
}


int main(int argc, char **argv)
{
    int fd;
    std::map<int, const char*> watching;

    if (init(argc, argv, fd, watching) != EXIT_SUCCESS) {
	return EXIT_FAILURE;
    }
    
    while (true) {
	if (print_changes(fd, watching) != EXIT_SUCCESS) {
	    return EXIT_FAILURE;
	}
    }
}
