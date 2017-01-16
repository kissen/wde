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

    static const size_t INITIAL_BUFSIZE = sizeof(struct inotify_event) + 256;
    size_t bufsize = INITIAL_BUFSIZE;

    while (true) {
	ssize_t rres;
	uint8_t buf[bufsize];

	// Try reading from the inotify socket

	rres = read(inotifyfd, buf, sizeof(buf));

	// Reading failed

	if (rres == -1) {
	    if (errno == EINVAL) {
		// EINVAL means the buffer was too small
		bufsize += 256;
		continue;
	    } else {
		fprintf(stderr, "%s: Cannot read ionotify descriptor: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	    }
	}

	// Reading succeeded
	// We should have gotten at least one event
	// XXX: What if we read too few bytes?

	struct inotify_event *event = (struct inotify_event *) buf;

	while (event != NULL) {
	    // len = 0 makes no sense because we are only watching dirs

	    if (event->len == 0) {
		fprintf(stderr, "%s: Unexpected ionotify behavior\n", argv[0]);
		return EXIT_FAILURE;
	    }

	    // Output the change

	    if (event->mask & IN_CREATE) {
		printf("-> %s\n", event->name);
	    } else if (event->mask & IN_DELETE) {
		printf("<- %s\n", event->name);
	    } else {
		fprintf(stderr, "%s: Unexpected ionotify behavior\n", argv[0]);
		return EXIT_FAILURE;
	    }

	    // There may still be another event hidden in that buffer!

	    event = NULL;
	}
    }
}
