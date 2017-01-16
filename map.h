/*
 * A very simple insert-only hash map that uses linear probing for
 * storing (int, const char *) pairs.
 */

#pragma once

struct map;

/*
 * Create a new map. Returns either a pointer to such a structure or
 * NULL on error, in which case errno is set.
 */
struct map *map_init(void);


/*
 * Free all resources allocated for map.
 */
void map_destroy(struct map *map);


/*
 * Insert the (key, value) pair into map. Returns 0 on success or -1
 * on failure, in which case errno is set.
 */
int map_insert(struct map *map, int key, const char *value);


/*
 * Get the value for key from map or NULL if no such element exists.
 */
const char *map_get(const struct map *map, int key);
