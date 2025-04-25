/**
 * @file hashdict_demo.c
 * @brief Demonstration program for hashdict library
 *
 * This program inserts a large number of key-value pairs into a hashdict
 * and prints the results.
 *
 * Copyright (c) 2025 Karsten Weikamp. All rights reserved.
 */

#include "hashdict.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Number of entries to insert
#define NUM_ENTRIES 1000

/**
 * @brief Generate a random string of specified length
 *
 * @param buffer Buffer to store the generated string
 * @param length Length of string to generate
 */
void
generate_random_string(char* buffer, int length) {
	static const char charset[] =
	    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	for (int i = 0; i < length - 1; i++) {
		int index = rand() % (sizeof(charset) - 1);
		buffer[i] = charset[index];
	}
	buffer[length - 1] = '\0';
}

int
main() {
	// Initialize random number generator
	srand(time(NULL));

	// Create a new dictionary
	struct hd_hashdict dict = hd_create();
	printf("Created new hashdict\n");

	// Insert a large number of key-value pairs
	char key_buffer[20];
	char value_buffer[100];
	int error_count = 0;

	printf("Inserting %d entries...\n", NUM_ENTRIES);

	for (int i = 0; i < NUM_ENTRIES; i++) {
		// Generate random key (5-15 chars)
		int key_length = 5 + (rand() % 11);
		generate_random_string(key_buffer, key_length);

		// Generate random value (10-80 chars)
		int value_length = 10 + (rand() % 71);
		generate_random_string(value_buffer, value_length);

		// Insert the key-value pair
		int result = hd_entry_insert(&dict, key_buffer, value_buffer);
		if (result != 0) {
			printf("Error inserting entry %d: %s\n", i, strerror(-result));
			error_count++;
		}
	}

	printf("\nInsertion complete. Errors: %d\n\n", error_count);

	// Print the dictionary contents
	printf("\nPrinting dictionary contents...\n\n");
	hd_print(&dict);

	// Free all allocated memory
	printf("\nFreeing dictionary...\n");
	hd_free(&dict);
	printf("Dictionary freed.\n");

	return 0;
}