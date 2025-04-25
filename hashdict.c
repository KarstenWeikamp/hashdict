#include "hashdict.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct hd_entry*
hd_lookup_entry(struct hd_hashdict* dict, const char* key);

/**
 * @brief Hash function for strings
 *
 * Implements the djb2 algorithm by Dan Bernstein, which is known for its
 * good distribution and speed for string keys.
 *
 * @param key The string to hash
 * @return unsigned int The hash value (modulo HASHSIZE)
 */
static unsigned int
hd_hash(const char* key) {
	unsigned long hash = 5381; // Magic starting number
	int c;

	while ((c = *key++)) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash % HASHSIZE;
}

struct hd_hashdict
hd_create(void) {
	struct hd_hashdict dict = {.num_entries = 0,
#ifdef DEBUG
	                           .collisions = 0,
	                           .alloced_bytes = 0
#endif /* DEBUG */
	};

	memset(dict.entries, 0, sizeof(dict.entries));
	return dict;
}

/**
 * @brief Free single-linked list of `hd_entry`s
 *
 * Recursively frees allocated memory of linked `hd_entry`s and sets the
 * pointers to NULL.
 * @sideeffects Modifies entry->key, entry->value, entry->next,
 * dict->num_entries
 */
static void
hd_free_entry_list(struct hd_entry* entry, struct hd_hashdict* dict) {
	/* assert here as were in trouble if this fails as we cant handle that error
	 * case.*/
	assert(dict != NULL);
	assert(entry != NULL);
	if (entry->next != NULL) {
		hd_free_entry_list(entry->next, dict);
		entry->next = NULL;
	}
#ifdef DEBUG
	dict->alloced_bytes -=
	    sizeof(entry->key) + sizeof(entry->value) + sizeof(struct hd_entry);
#endif /* DEBUG */
	free(entry->key);
	free(entry->value);
	free(entry);
	dict->num_entries--;
}

/**
 * @brief Frees all allocated memory from dict setting all freed pointers to
 * NULL.
 */
void
hd_free(struct hd_hashdict* dict) {
	if (dict == NULL) {
		return;
	}

	for (int i = 0; i < HASHSIZE; i++) {
		if (dict->entries[i] != NULL) {
			hd_free_entry_list(dict->entries[i], dict);
			dict->entries[i] = NULL;
		}
	}
#ifdef DEBUG
	printf("Allocated bytes after free: %d\nRemaining entries: %d\n",
	       dict->alloced_bytes, dict->num_entries);
#endif
}

/**
 * @brief Allocates memory for and copies str into it
 */
static char*
hd_stralloc(const char* str) {
	unsigned int str_len = strlen(str) + 1; // +1 for null terminator
	char* mem = malloc(str_len);
	memcpy(mem, str, str_len);
	return mem;
}

int
hd_entry_insert(struct hd_hashdict* dict, const char* key, const char* value) {
	if ((dict == NULL) || (key == NULL) ||
	    (hd_lookup_entry(dict, key) != NULL)) {
		return -EINVAL;
	}
	unsigned int hash = hd_hash(key);

	/* entry_ptr is a pointer to the address where the hd_entry should be
	 * allocated to in the end.*/
	struct hd_entry** entry_ptr = &(dict->entries[hash]);
	/* In case of a hash collision we iterate down the singly linked list to
	 * find a free spot*/
	if (*entry_ptr != NULL) {
#ifdef DEBUG
		dict->collisions++;
#endif /* DEBUG */
		while ((*entry_ptr)->next != NULL) {
			entry_ptr = &((*entry_ptr)->next);
		}
		/* Finally malloc hd_entry to its spot in hd_dict. */
		(*entry_ptr)->next = malloc(sizeof(struct hd_entry));
		entry_ptr = &((*entry_ptr)->next);
	} else {
		/* Simply allocate entry directly into hashlist if there is no
		 * collision*/
		dict->entries[hash] = malloc(sizeof(struct hd_entry));
		entry_ptr = &(dict->entries[hash]);
	}

	if (*entry_ptr == NULL) {
		return -ENOMEM;
	}

	(*entry_ptr)->key = hd_stralloc(key);

	if ((*entry_ptr)->key == NULL) {
		goto err_keyalloc;
	}

	(*entry_ptr)->value = hd_stralloc(value);

	if ((*entry_ptr)->value == NULL) {
		goto err_valalloc;
	}

	(*entry_ptr)->next = NULL;
	dict->num_entries++;

#ifdef DEBUG
	/* Only increase alloced_bytes if we know all allocs were successfull.
	 */
	dict->alloced_bytes += strlen((*entry_ptr)->key) + 1 +
	                       strlen((*entry_ptr)->value) + 1 +
	                       sizeof(struct hd_entry);
#endif /*DEBUG*/

	return 0;
err_valalloc:
	free((*entry_ptr)->key);
err_keyalloc:
	free(*entry_ptr);
	return -ENOMEM;
}

int
hd_entry_remove(struct hd_hashdict* dict, const char* key) {
	if ((dict == NULL) || (key == NULL)) {
		return -EINVAL;
	}

	if (dict->num_entries == 0) {
		return -EINVAL;
	}

	unsigned int hash = hd_hash(key);

	struct hd_entry** entry_ptr = &(dict->entries[hash]);

	/*Check if key exists in dict*/
	if (*entry_ptr == NULL) {
		return -EINVAL;
	}
	/* Store previous entry if entry is in a linked list because of hash
	 * collision if entry is at the beginning prev entry stays NULL.*/
	struct hd_entry* prev_entry = NULL;
	while (strcmp(key, (*entry_ptr)->key)) {
		if ((*entry_ptr)->next == NULL) {
			return -EINVAL;
		}
		prev_entry = *entry_ptr;
		entry_ptr = &((*entry_ptr)->next);
	}

	if (prev_entry == NULL) {
		/* If entry has a linked entry in next it will put in first place in
		 * entries if it doesn't it will be set to NULL so both cases are
		 * covered in this expression.*/
		dict->entries[hash] = (*entry_ptr)->next;
	} else {
		/* Here we use the same replacement mechanism to fixup the linked list
		 * as above.*/
		prev_entry->next = (*entry_ptr)->next;
	}

	dict->num_entries--;
#ifdef DEBUG
	dict->alloced_bytes -= strlen((*entry_ptr)->key) +
	                       strlen((*entry_ptr)->value) +
	                       sizeof(struct hd_entry);
#endif /*DEBUG*/

	free((*entry_ptr)->key);
	free((*entry_ptr)->value);
	free(*entry_ptr);
	return 0;
}

static struct hd_entry*
hd_lookup_entry(struct hd_hashdict* dict, const char* key) {
	if ((dict == NULL) || (key == NULL)) {
		return NULL;
	}

	if (dict->num_entries == 0) {
		return NULL;
	}

	unsigned int hash = hd_hash(key);

	struct hd_entry** entry_ptr = &(dict->entries[hash]);

	/*Check if key exists in dict*/
	if (*entry_ptr == NULL) {
		return NULL;
	}

	/* Check if current entrys key is actually the one we look for.
	 * If not, iterate over linked list in hashlist position.*/
	while (strcmp(key, (*entry_ptr)->key)) {
		if ((*entry_ptr)->next == NULL) {
			/*Key has a hash which has entries but is not actually in the
			 * list.*/
			return NULL;
		}
		entry_ptr = &((*entry_ptr)->next);
	}
	return (*entry_ptr);
}

const char*
hd_lookup(struct hd_hashdict* dict, const char* key) {
	struct hd_entry* entry = hd_lookup_entry(dict, key);
	return entry ? entry->value : NULL;
}

int
hd_entry_update(struct hd_hashdict* dict, const char* key, const char* value) {
	struct hd_entry* entry = hd_lookup_entry(dict, key);

	if (entry == NULL) {
		return -EINVAL;
	}

	char* new_value = hd_stralloc(value);

	if (new_value == NULL) {
		return -ENOMEM;
	}

#ifdef DEBUG
	/*Update allocated bytes with the difference of the new value and the old
	 * value*/
	dict->alloced_bytes += (strlen(new_value) - strlen(entry->value));
#endif /* DEBUG */

	free(entry->value);
	entry->value = new_value;

	return 0;
}

void
hd_print(struct hd_hashdict* dict) {
	if (dict == NULL) {
		printf("Dictionary is NULL\n");
		return;
	}

	// Fixed column widths
	const unsigned int key_width = 13;
	const unsigned int val_width = 48;
	const unsigned int idx_width = 6;

	// Print header with dictionary information
	printf("┌────────────────────────────────────────────────────────────┐\n");
	printf("│ Dictionary Statistics                                      │\n");
	printf("├────────────────────────────────────────────────────────────┤\n");
	printf("│ Total entries: %43u │\n", dict->num_entries);
#ifdef DEBUG
	printf("│ Collisions:    %43u │\n", dict->collisions);
	printf("│ Memory used:   %43d │\n", dict->alloced_bytes);
#endif /* DEBUG */
	printf(
	    "└────────────────────────────────────────────────────────────┘\n\n");

	if (dict->num_entries == 0) {
		printf("Dictionary is empty\n");
		return;
	}

	// Calculate total table width
	int table_width =
	    idx_width + key_width + val_width + 10; // 10 for borders and spaces

	// Print table header
	printf("┌");
	for (int i = 0; i < table_width - 2; i++)
		printf("─");
	printf("┐\n");

	printf("│ %-*s │ %-*s │ %-*s │\n", idx_width, "Bucket", key_width, "Key",
	       val_width, "Value");

	printf("├");
	for (unsigned int i = 0; i < idx_width + 2; i++)
		printf("─");
	printf("┼");
	for (unsigned int i = 0; i < key_width + 2; i++)
		printf("─");
	printf("┼");
	for (unsigned int i = 0; i < val_width + 2; i++)
		printf("─");
	printf("┤\n");

	// Print table contents
	char key_buf[key_width + 1];
	char val_buf[val_width + 1];

	for (int i = 0; i < HASHSIZE; i++) {
		struct hd_entry* entry = dict->entries[i];
		if (entry != NULL) {
			while (entry != NULL) {
				// Format key (truncate if needed)
				if (strlen(entry->key) > key_width - 4) {
					strncpy(key_buf, entry->key, key_width - 4);
					strcpy(key_buf + key_width - 4, "...");
				} else {
					strcpy(key_buf, entry->key);
				}

				// Format value (truncate if needed)
				if (strlen(entry->value) > val_width - 4) {
					strncpy(val_buf, entry->value, val_width - 4);
					strcpy(val_buf + val_width - 4, "...");
				} else {
					strcpy(val_buf, entry->value);
				}

				printf("│ %-*d │ %-*s │ %-*s │\n", idx_width, i, key_width,
				       key_buf, val_width, val_buf);

				entry = entry->next;
			}
		}
	}

	// Print table footer
	printf("└");
	for (unsigned int i = 0; i < idx_width + 2; i++)
		printf("─");
	printf("┴");
	for (unsigned int i = 0; i < key_width + 2; i++)
		printf("─");
	printf("┴");
	for (unsigned int i = 0; i < val_width + 2; i++)
		printf("─");
	printf("┘\n");
}
