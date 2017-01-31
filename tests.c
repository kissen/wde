#include <check.h>
#include "map.h"


/*
 * Tests for map.c
 */


static const char *DUMMY_VALUE = (char *) 0xbadf00d;


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
	ck_assert(map_insert(m, i, DUMMY_VALUE + i) != -1);
    }

    ck_assert(map_items(m) == 25);
    map_destroy(m);
}
END_TEST


START_TEST(map_insert_items_many)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 1000; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUE + i) != -1);
    }

    ck_assert(map_items(m) == 1000);
    map_destroy(m);
}
END_TEST


START_TEST(map_insert_removeall_items)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 25; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUE + i) != -1);
    }

    for (int i = 0; i < 25; ++i) {
	ck_assert(map_remove(m, i) != NULL);
    }

    ck_assert(map_items(m) == 0);
    map_destroy(m);
}
END_TEST


START_TEST(map_insert_removeall_items_many)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 1000; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUE + i) != -1);
    }

    for (int i = 0; i < 1000; ++i) {
	ck_assert(map_remove(m, i) != NULL);
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
	ck_assert(map_insert(m, i, DUMMY_VALUE + i) != -1);
    }

    for (int i = 0; i < 100; ++i) {
	ck_assert(map_get(m, i) == DUMMY_VALUE + i);
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
	ck_assert(map_insert(m, i, DUMMY_VALUE + i) != -1);
    }

    for (int i = 0; i < 100; ++i) {
	ck_assert(map_remove(m, i) != NULL);
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
	ck_assert(map_insert(m, i, DUMMY_VALUE + i) != -1);
    }

    for (int i = 0; i < 100; ++i) {
	if (i % 3 == 0) {
	    ck_assert(map_remove(m, i) != NULL);
	}
    }

    for (int i = 0; i < 100; ++i) {
	if (i % 3 == 0) {
	    ck_assert(map_get(m, i) == NULL);
	} else {
	    ck_assert(map_get(m, i) == DUMMY_VALUE + i);
	}
    }

    map_destroy(m);
}
END_TEST


START_TEST(map_insert_removesome_insert_get)
{
    struct map *m = map_init();
    ck_assert(m != NULL);

    for (int i = 0; i < 1000; ++i) {
	ck_assert(map_insert(m, i, DUMMY_VALUE + i) != -1);
    }

    for (int i = 0; i < 1000; ++i) {
	if (i % 3 == 0) {
	    ck_assert(map_remove(m, i) != NULL);
	}
    }

    static const char *dummy_other = (char *) 0xbadcafe;

    for (int i = 0; i < 1000; ++i) {
	if (i % 2 == 0) {
	    ck_assert(map_insert(m, i, dummy_other + i) != -1);
	}
    }

    for (int i = 0; i < 1000; ++i) {
	if (i % 2 == 0) {
	    ck_assert(map_get(m, i) == dummy_other + i);
	} else 	if (i % 3 == 0) {
	    ck_assert(map_get(m, i) == NULL);
	} else {
	    ck_assert(map_get(m, i) == DUMMY_VALUE + i);
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

    init(1, argv, &fd, &watching);
}
END_TEST


static Suite *wde_suite(void)
{
    Suite *s = suite_create("wde");
    TCase *wde_tc = tcase_create("core");

    tcase_add_test(wde_tc, wde_init_cmd);
    tcase_add_exit_test(wde_tc, wde_empty_args, 1);

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
