#pragma once

#include <stdint.h>


typedef struct HashEntry
{
	const char* Key;
	void* Value;
	uint32_t Next;
}
HashEntry;

typedef struct HashTable
{
	uint32_t* Buckets;
	HashEntry* Entries;

	uint32_t BucketCount;
	uint32_t EntriesUsed;
	uint32_t EntriesSize;
	uint32_t FreeEntry;
}
HashTable;


extern void
HashInit(
	HashTable* Table,
	uint32_t BucketCount
	);


extern void
HashFree(
	HashTable* Table
	);


extern void
HashSet(
	HashTable* Table,
	const char* Key,
	void* Value
	);


extern void*
HashGet(
	HashTable* Table,
	const char* Key
	);


extern void
HashRemove(
	HashTable* Table,
	const char* Key
	);
