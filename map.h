/*
 * A very simple hash map that uses linear probing for storing
 * (int, null-terminated string) pairs.
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
 * Return the number of items stored in map.
 */
size_t map_items(const struct map *map);


/*
 * Insert the (key, value) pair into map. value needs to be a
 * null-terminated string and will be copied for the hash map. If a
 * pair with this key already exists, that pair stored in map is
 * overwritten. Returns 0 on success or -1 on failure, in which case
 * errno is set.
 */
int map_insert(struct map *map, int key, const char *value);


/*
 * Get the value for key from map or NULL if no such element exists.
 * Do not free the returned pointer, use map_remove() instead.
 */
const char *map_get(const struct map *map, int key);


/*
 * Remove the value for key from map. Memory allocated for that pair
 * will be freed. Returns 0 on success and -1 if no such element could
 * be found.
 */
const int map_remove(struct map *map, int key);
