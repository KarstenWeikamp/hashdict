A memory-efficient, fast lookup dictionary implementation based on the 
hashlist pattern described in Chapter 6.6 of "The C Programming Language"
by Brian W. Kernighan and Dennis M. Ritchie.
 
This implementation provides O(1) average case lookup times by using
a hash function to distribute keys across buckets, with collision
resolution via chaining (linked lists).
 
IMPORTANT: This implementation is _not_ thread safe. When using it in a
multithreaded enviroment with multiple writing threads,
the user has to handle synchronisiation.

<Copyright (c) 2025 Karsten Weikamp. All rights reserved.>
