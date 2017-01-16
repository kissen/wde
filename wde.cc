#include <sys/inotify.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <map>


int main(int argc, char **argv)
{
    int inotifyfd;
    std::map <int, const char*> watched;

    if (argc <= 1) {
	fprintf(stderr, "%s: Missing arguments\n", argv[0]);
	return EXIT_FAILURE;
    }

    // Initialize inotify

    if ((inotifyfd = inotify_init()) == -1) {
	fprintf(stderr, "%s: Init failed: %s\n", argv[0], strerror(errno));
	return EXIT_FAILURE;
    }

    // Add all directories

    for (int i = 1; i < argc; ++i) {
	int watchfd;
	uint32_t mask = IN_CREATE | IN_DELETE | IN_ONLYDIR;

	if ((watchfd = inotify_add_watch(inotifyfd, argv[i], mask)) == -1) {
	    fprintf(stderr, "%s: Adding %s failed: %s\n", argv[0], argv[i], strerror(errno));
	    return EXIT_FAILURE;
	}

	watched.insert(std::make_pair(watchfd, argv[i]));
    }

    // Now keep reporting changes

    while (true) {
	ssize_t res;

	// First, read the header of the event. The length of each of those events is actually
	// different because it contains the name[] field which has dynamic length.

	struct {
	    int wd;
	    uint32_t mask;
	    uint32_t cookie;
	    uint32_t len;
	} header;

	res = read(inotifyfd, &header, sizeof(header) + 200);

	if (res == -1) {
	    fprintf(stderr, "%s: Cannot read ionotify descriptor: %s\n", argv[0], strerror(errno));
	    return EXIT_FAILURE;
	} else if ((size_t) res < sizeof(header) || header.len == 0) {
	    fprintf(stderr, "%s: Unexpected ionotify behavior\n", argv[0]);
	    return EXIT_FAILURE;
	}

	// Now that we have the header we can read the name[] field

	char name[header.len];  // XXX: Dynamic Array

	res = read(inotifyfd, name, sizeof(name));

	if (res == -1) {
	    fprintf(stderr, "%s: Cannot read ionotify descriptor: %s\n", argv[0], strerror(errno));
	    return EXIT_FAILURE;
	} else if ((size_t) res < sizeof(name)) {
	    fprintf(stderr, "%s: Unexpected ionotify behavior\n", argv[0]);
	    return EXIT_FAILURE;
	}

	// Inform user of change

	if (header.mask & IN_CREATE) {
	    printf("-> %s\n", name);
	} else if (header.mask & IN_DELETE) {
	    printf("<- %s\n", name);
	} else {
	    fprintf(stderr, "%s: Unexpected ionotify behavior\n", argv[0]);
	    return EXIT_FAILURE;
	}
    }
}
