#include <check.h>
#include "map.h"


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


int main(void)
{
    int failed;

    Suite *s;
    SRunner *r;
    TCase *tc;

    s = suite_create("map");
    tc = tcase_create("core");

    // add tests cases
    tcase_add_test(tc, map_create_destroy);
    tcase_add_test(tc, map_insert_items);
    tcase_add_test(tc, map_insert_items_many);
    tcase_add_test(tc, map_insert_removeall_items);
    tcase_add_test(tc, map_insert_removeall_items_many);
    tcase_add_test(tc, map_getempty);
    tcase_add_test(tc, map_insert_get);
    tcase_add_test(tc, map_insert_removeall_get);
    tcase_add_test(tc, map_insert_removesome_get);
    tcase_add_test(tc, map_insert_removesome_insert_get);

    suite_add_tcase(s, tc);
    r = srunner_create(s);

    srunner_run_all(r, CK_NORMAL);
    failed = srunner_ntests_failed(r);
    srunner_free(r);
    return failed;
}
