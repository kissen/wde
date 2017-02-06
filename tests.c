#include <check.h>
#include <fcntl.h>
#include "map.h"


/*
 * Tests for map.c
 */


static char *DUMMY_VALUES[] = {
    "string0", "string1", "string2", "string3", "string4", "string5",
    "string6", "string7", "string8", "string9", "string10",
    "string11", "string12", "string13", "string14", "string15",
    "string16", "string17", "string18", "string19", "string20",
    "string21", "string22", "string23", "string24", "string25",
    "string26", "string27", "string28", "string29", "string30",
    "string31", "string32", "string33", "string34", "string35",
    "string36", "string37", "string38", "string39", "string40",
    "string41", "string42", "string43", "string44", "string45",
    "string46", "string47", "string48", "string49", "string50",
    "string51", "string52", "string53", "string54", "string55",
    "string56", "string57", "string58", "string59", "string60",
    "string61", "string62", "string63", "string64", "string65",
    "string66", "string67", "string68", "string69", "string70",
    "string71", "string72", "string73", "string74", "string75",
    "string76", "string77", "string78", "string79", "string80",
    "string81", "string82", "string83", "string84", "string85",
    "string86", "string87", "string88", "string89", "string90",
    "string91", "string92", "string93", "string94", "string95",
    "string96", "string97", "string98", "string99"
};

static char *DUMMY_OTHER[] = {
    "other0", "other1", "other2", "other3", "other4", "other5",
    "other6", "other7", "other8", "other9", "other10", "other11",
    "other12", "other13", "other14", "other15", "other16", "other17",
    "other18", "other19", "other20", "other21", "other22", "other23",
    "other24", "other25", "other26", "other27", "other28", "other29",
    "other30", "other31", "other32", "other33", "other34", "other35",
    "other36", "other37", "other38", "other39", "other40", "other41",
    "other42", "other43", "other44", "other45", "other46", "other47",
    "other48", "other49", "other50", "other51", "other52", "other53",
    "other54", "other55", "other56", "other57", "other58", "other59",
    "other60", "other61", "other62", "other63", "other64", "other65",
    "other66", "other67", "other68", "other69", "other70", "other71",
    "other72", "other73", "other74", "other75", "other76", "other77",
    "other78", "other79", "other80", "other81", "other82", "other83",
    "other84", "other85", "other86", "other87", "other88", "other89",
    "other90", "other91", "other92", "other93", "other94", "other95",
    "other96", "other97", "other98", "other99"
};


START_TEST(map_create_destroy)
{
    struct map *m = map_init();
    ck_assert(m != NULL);
    map_destroy(m);
}
END_TEST


START_TEST(map_insert_items)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 25; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUES[i]) != -1);
    }

    ck_assert(map_items(m) == 25);
    map_destroy(m);
}
END_TEST


START_TEST(map_insert_items_many)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 100; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUES[i]) != -1);
    }

    ck_assert(map_items(m) == 100);
    map_destroy(m);
}
END_TEST


START_TEST(map_insert_removeall_items)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 25; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUES[i]) != -1);
    }

    for (int i = 0; i < 25; ++i) {
	ck_assert(map_remove(m, i) != -1);
    }

    ck_assert(map_items(m) == 0);
    map_destroy(m);
}
END_TEST


START_TEST(map_insert_removeall_items_many)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 100; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUES[i]) != -1);
    }

    for (int i = 0; i < 100; ++i) {
	ck_assert(map_remove(m, i) != -1);
    }

    ck_assert(map_items(m) == 0);
    map_destroy(m);
}
END_TEST


START_TEST(map_insert_get)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 100; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUES[i]) != -1);
    }

    for (int i = 0; i < 100; ++i) {
	ck_assert_str_eq(map_get(m, i), DUMMY_VALUES[i]);
    }

    map_destroy(m);
}
END_TEST


START_TEST(map_getempty)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = -64; i <= 64; ++i) {
	ck_assert(map_get(m, i) == NULL);
    }

    map_destroy(m);
}
END_TEST


START_TEST(map_insert_removeall_get)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 100; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUES[i]) != -1);
    }

    for (int i = 0; i < 100; ++i) {
	ck_assert(map_remove(m, i) != -1);
    }

    for (int i = 0; i < 100; ++i) {
	ck_assert(map_get(m, i) == NULL);
    }

    map_destroy(m);
}
END_TEST


START_TEST(map_insert_removesome_get)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 100; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUES[i]) != -1);
    }

    for (int i = 0; i < 100; ++i) {
	if (i % 3 == 0) {
	    ck_assert(map_remove(m, i) != -1);
	}
    }

    for (int i = 0; i < 100; ++i) {
	if (i % 3 == 0) {
	    ck_assert(map_get(m, i) == NULL);
	} else {
	    ck_assert_str_eq(map_get(m, i), DUMMY_VALUES[i]);
	}
    }

    map_destroy(m);
}
END_TEST


START_TEST(map_insert_removesome_insert_get)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 100; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUES[i]) != -1);
    }

    for (int i = 0; i < 100; ++i) {
	if (i % 3 == 0) {
	    ck_assert(map_remove(m, i) != -1);
	}
    }

    for (int i = 0; i < 100; ++i) {
	if (i % 2 == 0) {
	    ck_assert(map_insert(m, i, DUMMY_OTHER[i]) != -1);
	}
    }

    for (int i = 0; i < 100; ++i) {
	if (i % 2 == 0) {
	    ck_assert_str_eq(map_get(m, i), DUMMY_OTHER[i]);
	} else 	if (i % 3 == 0) {
	    ck_assert(map_get(m, i) == NULL);
	} else {
	    ck_assert_str_eq(map_get(m, i), DUMMY_VALUES[i]);
	}
    }

    map_destroy(m);
}
END_TEST


static Suite *map_suite(void)
{
    Suite *s = suite_create("map");
    TCase *map_tc = tcase_create("core");

    tcase_add_test(map_tc, map_create_destroy);
    tcase_add_test(map_tc, map_insert_items);
    tcase_add_test(map_tc, map_insert_items_many);
    tcase_add_test(map_tc, map_insert_removeall_items);
    tcase_add_test(map_tc, map_insert_removeall_items_many);
    tcase_add_test(map_tc, map_getempty);
    tcase_add_test(map_tc, map_insert_get);
    tcase_add_test(map_tc, map_insert_removeall_get);
    tcase_add_test(map_tc, map_insert_removesome_get);
    tcase_add_test(map_tc, map_insert_removesome_insert_get);

    suite_add_tcase(s, map_tc);
    return s;
}


/*
 * Tests for wde.c
 */

#define main wde_main
#include "wde.c"
#undef main


/*
 * Forward stdout and stderr to /dev/null.
 */
static void suppress_output(void)
{
    int fd = open("/dev/null", O_RDWR);
    ck_assert(fd != 1);

    ck_assert(dup2(fd, STDOUT_FILENO) != -1);
    ck_assert(dup2(fd, STDERR_FILENO) != -1);
}


/*
 * Create a temp directory and write the path into buf of buflen
 * capacity.
 */
static void create_temp_dir(void *buf, size_t buflen)
{
    snprintf(buf, buflen, "/tmp/wdetestXXXXXX");
    ck_assert(mkdtemp(buf) != NULL);
}


/*
 * Create a file named filename in dir. Note:
 *
 *     strlen(dir) + strlen(filename) + 2
 *
 * has to be smaller than 1024.
 */
static void create_dummy_file(const char *dir, const char *filename)
{
    static char buf[1024];

    snprintf(buf, sizeof(buf), "%s/%s", dir, filename);
    FILE *fp = fopen(buf, "w+");
    ck_assert(fp != NULL);
    ck_assert(fclose(fp) == 0);
}


START_TEST(wde_init_cmd)
{
    char *argv[] = {
	"progname",
	"/tmp",
	NULL
    };

    int fd;
    struct map *watching;

    init(2, argv, &fd, &watching);
    ck_assert(cmd == argv[0]);
}
END_TEST


START_TEST(wde_empty_args)
{
    char *argv[] = {
	"wde",
	NULL
    };

    int fd;
    struct map *watching;

    suppress_output();
    init(1, argv, &fd, &watching);
}
END_TEST


START_TEST(wde_add_and_remove_dirs)
{
    static char dir0[1024];
    static char dir1[1024];

    create_temp_dir(dir0, sizeof(dir0));
    create_temp_dir(dir1, sizeof(dir1));

    char *argv[] = {
	"wde",
	dir0,
	dir1,
	NULL
    };

    int fd;
    struct map *watching;

    suppress_output();
    init(3, argv, &fd, &watching);
    ck_assert(map_items(watching) == 2);

    ck_assert(rmdir(dir0) == 0);
    ck_assert(rmdir(dir1) == 0);
    sleep(1);

    read_and_print(fd, watching);
    ck_assert(map_items(watching) == 0);
}
END_TEST


START_TEST(wde_add_files)
{
    static char dir0[1024];
    static char dir1[1024];

    create_temp_dir(dir0, sizeof(dir0));
    create_temp_dir(dir1, sizeof(dir1));

    char *argv[] = {
	"wde",
	dir0,
	dir1,
	NULL
    };

    int fd;
    struct map *watching;

    suppress_output();
    init(3, argv, &fd, &watching);
    ck_assert(map_items(watching) == 2);

    create_dummy_file(dir0, "file0");
    create_dummy_file(dir1, "file1");
    create_dummy_file(dir0, "file2");
    create_dummy_file(dir1, "file3");
    sleep(1);

    ck_assert(read_and_print(fd, watching) == 4);
}
END_TEST


START_TEST(wde_handle_event)
{
    // Redirect stdout

    int fd[2];
    ck_assert(pipe(fd) == 0);

    int w = fd[1];
    int r = fd[0];

    ck_assert(dup2(w, STDOUT_FILENO) != -1);

    // Create matching structures and associated map

    static const struct inotify_event event = {
	.wd = 100,
	.mask = IN_CREATE,
	.len = 15,
	.name = "abyz"
    };

    struct map *map = map_init();
    ck_assert(map != NULL);
    ck_assert(map_insert(map, event.wd, "1234") == 0);

    // Check if handle_event reacts correctly

    handle_event(&event, map);
    static char outbuf[1024];
    ck_assert(read(r, outbuf, sizeof(outbuf)) > 0);
    outbuf[sizeof(outbuf) - 1] = 0;
    ck_assert(strstr(outbuf, "-> 1234/abyz") != NULL);
}
END_TEST


static Suite *wde_suite(void)
{
    Suite *s = suite_create("wde");
    TCase *wde_tc = tcase_create("core");

    tcase_add_test(wde_tc, wde_init_cmd);
    tcase_add_exit_test(wde_tc, wde_empty_args, 1);
    tcase_add_test(wde_tc, wde_add_and_remove_dirs);
    tcase_add_test(wde_tc, wde_add_files);
    tcase_add_test(wde_tc, wde_handle_event);

    suite_add_tcase(s, wde_tc);
    return s;
}


/*
 * Running tests
 */


int main(void)
{
    Suite *ms = map_suite();
    Suite *ws = wde_suite();

    SRunner *r = srunner_create(ms);
    srunner_add_suite(r, ws);

    srunner_run_all(r, CK_NORMAL);
    int failed = srunner_ntests_failed(r);
    srunner_free(r);

    return failed == 0 ? 0 : 1;
}
