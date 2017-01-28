#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include "map.h"


static const size_t INITIAL_CAPACITY = 32;


struct map_entry {
    int key;
    const char *value;
    bool used;
};


struct map {
    size_t capacity;
    size_t items;
    struct map_entry *data;
};


static int hash(const struct map *map, int key)
{
    return abs(key) % map->capacity;
}


/*
 * Double the capacity of map. Returns 0 on success.
 * Runs in O(map->capacity).
 */
static int increase_size(struct map *map)
{
    struct map new = *map;

    new.capacity *= 2;
    new.items = 0;

    if ((new.data = calloc(new.capacity, sizeof(struct map_entry))) == NULL) {
	return -1;
    }

    for (size_t i = 0; i < map->capacity; ++i) {
	struct map_entry *entry = &map->data[i];

	if (entry->used) {
	    if (map_insert(&new, entry->key, entry->value) == -1) {
		free(new.data);
		return -1;
	    }
	}
    }

    free(map->data);
    *map = new;

    return 0;
}


struct map *map_init(void)
{
    struct map *map;

    if ((map = malloc(sizeof(*map))) == NULL) {
	return NULL;
    }

    map->capacity = INITIAL_CAPACITY;
    map->items = 0;
    if ((map->data = calloc(map->capacity, sizeof(struct map_entry))) == NULL) {
	free(map);
	return NULL;
    }

    return map;
}


void map_destroy(struct map *map)
{
    free(map->data);
    free(map);
}


int map_insert(struct map *map, int key, const char *value)
{
    // If there is no more space, allocate more memory

    if (map->capacity <= map->items) {
	if (increase_size(map) != 0) {
	    return -1;
	}
    }

    // Insert key/value pair

    int i = hash(map, key);

    do {
	if (! map->data[i].used || map->data[i].key == key) {
	    map->data[i].key = key;
	    map->data[i].value = value;
	    map->data[i].used = true;

	    map->items += 1;
	    break;
	}

	i = (i + 1) % map->capacity;
    } while (i != hash(map, key));

    return 0;
}


const char *map_get(const struct map *map, int key)
{
    int i = hash(map, key);

    do {
	if (map->data[i].used && map->data[i].key == key) {
	    return map->data[i].value;
	}

	if (! map->data[i].used) {
	    return NULL;
	}

	i = (i + 1) % map->capacity;
    } while (i != hash(map, key));

    return NULL;
}
