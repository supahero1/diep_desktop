#include <DiepDesktop/shared/hash.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>

#include <string.h>


Static uint32_t
FNV_1a(
	const char* Key
	)
{
	uint32_t Hash = 0x811c9dc5;

	while(*Key)
	{
		Hash ^= *(Key++);
		Hash *= 0x01000193;
	}

	return Hash;
}


void
HashInit(
	HashTable* Table,
	uint32_t BucketCount
	)
{
	Table->BucketCount = BucketCount;
	AssertNEQ(Table->BucketCount, 0);

	Table->EntriesUsed = 1;
	Table->EntriesSize = 0;
	Table->FreeEntry = 0;

	Table->Buckets = AllocCalloc(sizeof(uint32_t) * Table->BucketCount);
	AssertNotNull(Table->Buckets);

	Table->Entries = NULL;
}


void
HashFree(
	HashTable* Table
	)
{
	AllocFree(sizeof(uint32_t) * Table->BucketCount, Table->Buckets);
	AllocFree(sizeof(HashEntry) * Table->EntriesSize, Table->Entries);
}


Static uint32_t
HashGetEntry(
	HashTable* Table
	)
{
	if(Table->FreeEntry)
	{
		uint32_t Entry = Table->FreeEntry;
		Table->FreeEntry = Table->Entries[Entry].Next;
		return Entry;
	}

	if(Table->EntriesUsed >= Table->EntriesSize)
	{
		uint32_t NewSize = (Table->EntriesUsed << 1) | 1;
		HashEntry* NewEntries = AllocRemalloc(
			sizeof(HashEntry) * Table->EntriesSize,
			Table->Entries,
			sizeof(HashEntry) * NewSize
			);
		AssertNotNull(NewEntries);

		Table->Entries = NewEntries;
		Table->EntriesSize = NewSize;
	}

	return Table->EntriesUsed++;
}


Static void
HashRetEntry(
	HashTable* Table,
	uint32_t Entry
	)
{
	Table->Entries[Entry].Next = Table->FreeEntry;
	Table->FreeEntry = Entry;
}


void
HashSet(
	HashTable* Table,
	const char* Key,
	void* Value
	)
{
	uint32_t Hash = FNV_1a(Key) % Table->BucketCount;

	uint32_t* Next = &Table->Buckets[Hash];
	for(uint32_t Entry = *Next; Entry; Entry = Table->Entries[Entry].Next)
	{
		if(strcasecmp(Key, Table->Entries[Entry].Key) == 0)
		{
			Table->Entries[Entry].Value = Value;
			return;
		}

		Next = &Table->Entries[Entry].Next;
	}

	uint32_t Entry = HashGetEntry(Table);
	Table->Entries[Entry] =
	(HashEntry)
	{
		.Key = Key,
		.Value = Value,
		.Next = 0
	};

	*Next = Entry;
}


void*
HashGet(
	HashTable* Table,
	const char* Key
	)
{
	uint32_t Hash = FNV_1a(Key) % Table->BucketCount;

	for(uint32_t Entry = Table->Buckets[Hash]; Entry; Entry = Table->Entries[Entry].Next)
	{
		if(strcasecmp(Key, Table->Entries[Entry].Key) == 0)
		{
			return Table->Entries[Entry].Value;
		}
	}

	return NULL;
}


void
HashRemove(
	HashTable* Table,
	const char* Key
	)
{
	uint32_t Hash = FNV_1a(Key) % Table->BucketCount;

	uint32_t* Next = &Table->Buckets[Hash];
	for(uint32_t Entry = *Next; Entry; Entry = Table->Entries[Entry].Next)
	{
		if(strcasecmp(Key, Table->Entries[Entry].Key) == 0)
		{
			*Next = Table->Entries[Entry].Next;
			HashRetEntry(Table, Entry);
			break;
		}

		Next = &Table->Entries[Entry].Next;
	}
}
