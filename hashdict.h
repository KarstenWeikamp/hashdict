/**
 * @file hashdict.h
 * @brief Hash table-based dictionary implementation for string keys and values
 *
 * A memory-efficient, fast lookup dictionary implementation based on the
 * hashlist pattern described in Chapter 6.6 of "The C Programming Language"
 * by Brian W. Kernighan and Dennis M. Ritchie.
 *
 * This implementation provides O(1) average case lookup times by using
 * a hash function to distribute keys across buckets, with collision
 * resolution via chaining (linked lists).
 *
 * IMPORTANT: This implementation is _not_ thread safe. When using it in a
 * multithreaded enviroment with multiple writing threads,
 * the user has to handle synchronisiation.
 *
 * Copyright (c) 2025 Karsten Weikamp. All rights reserved.
 */

#ifndef HASHDICT_H
#define HASHDICT_H

#define HASHSIZE 1024 /**< Number of hash buckets in the table */

#define DEBUG
/**
 * @brief Hash table entry structure
 *
 * Each entry contains a key-value pair (strings) and a pointer to the next
 * entry in case of hash collisions.
 */
struct hd_entry {
	struct hd_entry* next; /**< Pointer to next entry (NULL if none) */
	char* key; /**< String key (dynamically allocated copy) */
	char* value; /**< String value (dynamically allocated copy) */
};

/**
 * @brief Hash dictionary structure
 *
 * Contains the hash table (array of entry pointers), entry count,
 * and optional debug information.
 */
struct hd_hashdict {
	struct hd_entry* entries[HASHSIZE]; /**< Array of hash buckets */
	unsigned int num_entries; /**< Total number of entries in dictionary */
#ifdef DEBUG
	unsigned int collisions; /**< Number of hash collisions (debug only) */
	int alloced_bytes; /**< Total memory allocated (debug only) */
#endif /* DEBUG */
};

/**
 * @brief Create a new empty hash dictionary
 *
 * @return struct hd_hashdict An initialized empty dictionary
 */
struct hd_hashdict
hd_create(void);

/**
 * @brief Free all memory associated with a dictionary
 *
 * Releases all entries, keys, and values, setting all freed pointers to NULL.
 *
 * @param dict Pointer to the dictionary to free
 */
void
hd_free(struct hd_hashdict* dict);

/**
 * @brief Insert a new key-value pair into the dictionary
 *
 * Creates copies of both key and value strings. If the key already exists,
 * it will be added again with potential duplicate keys.
 *
 * @param dict Pointer to the dictionary
 * @param key String key to insert (must not be NULL)
 * @param value String value to associate with the key
 * @return int 0 on success, -EINVAL for invalid parameters, -ENOMEM if out of
 * memory
 */
int
hd_entry_insert(struct hd_hashdict* dict, const char* key, const char* value);

/**
 * @brief Remove an entry from the dictionary by key
 *
 * Removes the first occurrence of the specified key.
 *
 * @param dict Pointer to the dictionary
 * @param key Key to remove
 * @return int 0 on success, -EINVAL if key not found or invalid parameters
 */
int
hd_entry_remove(struct hd_hashdict* dict, const char* key);

/**
 * @brief Look up a value by key
 *
 * @param dict Pointer to the dictionary
 * @param key Key to look up
 * @return const char* The value associated with the key, or NULL if not found
 */
const char*
hd_lookup(struct hd_hashdict* dict, const char* key);

/**
 * @brief Update the value associated with a key
 *
 * @param dict Pointer to the dictionary
 * @param key Key to update (must exist)
 * @param value New value to associate with the key
 * @return int 0 on success, -EINVAL if key not found, -ENOMEM if out of memory
 */
int
hd_entry_update(struct hd_hashdict* dict, const char* key, const char* value);

/**
 * @brief Print a formatted representation of the dictionary
 *
 * Displays dictionary statistics and contents in a tabular format.
 * Keys longer than 13 characters and values longer than 48 characters
 * are truncated with ellipses.
 *
 * @param dict Pointer to the dictionary to print
 */
void
hd_print(struct hd_hashdict* dict);

#endif /* HASHDICT_H */