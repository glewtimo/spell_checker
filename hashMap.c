#define _CRT_SECURE_NO_WARNINGS

#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>


int hashFunction1(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += key[i];
    }
    return r;
}

int hashFunction2(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += (i + 1) * key[i];
    }
    return r;
}

/**
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink* hashLinkNew(const char* key, int value, HashLink* next)
{
    HashLink* link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}

/**
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link
 */
static void hashLinkDelete(HashLink* link)
{
    free(link->key);
    free(link);
}

/**
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap* map, int capacity)
{
    map->capacity = capacity;
    map->size = 0;
    map->table = malloc(sizeof(HashLink*) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        map->table[i] = NULL;
    }
}

/**
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param map
 */
void hashMapCleanUp(HashMap* map)
{
    for(int i = map->capacity - 1; i >= 0; i--){
        HashLink* currLink = map->table[i];

		//Free links in each bucket
		while (currLink != 0) {
			HashLink* temp;
			temp = currLink;
			currLink = currLink->next;
			hashLinkDelete(temp);
			map->size--;
		}
    }
	//Free buckets
	free(map->table);
}

/**
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap* hashMapNew(int capacity)
{
    HashMap* map = malloc(sizeof(HashMap));
    hashMapInit(map, capacity);
    return map;
}

/**
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map
 */
void hashMapDelete(HashMap* map)
{
    hashMapCleanUp(map);
    free(map);
}

/**
 * Returns a pointer to the value of the link with the given key and skip traversing as well. Returns NULL
 * if no link with that key is in the table.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return Link value or NULL if no matching link.
 */
int* hashMapGet(HashMap* map, const char* key)
{
    int hashIndex = HASH_FUNCTION(key) % map->capacity;
    int* found = NULL;
    HashLink* currLink = map->table[hashIndex];
    while(currLink != 0){
        if(!strcmp(currLink->key, key)){
            found = &(currLink->value);
        }
        currLink = currLink->next;
    }

    return found;
}

/**
 * Resizes the hash table to have a number of buckets equal to the given 
 * capacity (double of the old capacity). After allocating the new table, 
 * all of the links need to rehashed into it because the capacity has changed.
 * 
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 * 
 * @param map
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap* map, int capacity)
{
    //Create new map with new capacity
    HashMap* newMap = hashMapNew(capacity);

    //Copy values from old table into new table
    for(int i = 0; i < map->capacity; i++){
        HashLink* currLink = map->table[i];
        while(currLink != NULL){
            hashMapPut(newMap, currLink->key, currLink->value);
            currLink = currLink->next;
        }
    }

    //Flip tables so old map points to new table and new map points to old table
    HashLink** temp = map->table;
    map->table = newMap->table;
    newMap->table = temp;

    //Delete new map, which will also delete the old table
	newMap->capacity = map->capacity;
    hashMapDelete(newMap);

	//Update capacity of old map
	map->capacity = capacity;
}

/**
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value and skip traversing. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket.
 * 
 * @param map
 * @param key
 * @param value
 */
void hashMapPut(HashMap* map, const char* key, int value)
{
    int* linkVal = hashMapGet(map, key);

    if(linkVal == NULL){
        //Check if table needs to be resized
        if(hashMapTableLoad(map) >= MAX_TABLE_LOAD){
            resizeTable(map, map->capacity*2);
        }

        int hashIndex = HASH_FUNCTION(key) % map->capacity;
        HashLink* next = map->table[hashIndex];
        HashLink* newLink = hashLinkNew(key, value, next);
        map->table[hashIndex] = newLink;
        map->size++;
    }
    else{
        *linkVal = value;
    }  
}

/**
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map
 * @param key
 */
void hashMapRemove(HashMap* map, const char* key)
{
    int hashIndex = HASH_FUNCTION(key) % map->capacity;
    HashLink* currLink = map->table[hashIndex];

	//If the link to be removed is the first link, update map->table to point
	//to second element before removal
	if (!strcmp(currLink->key, key)) {
		map->table[hashIndex] = currLink->next;
		hashLinkDelete(currLink);
		map->size--;
		return;
	}


	HashLink* prevLink = currLink;
	currLink = currLink->next;
    while(currLink != 0){
        if(!strcmp(currLink->key, key)){
			prevLink->next = currLink->next;
            hashLinkDelete(currLink);
			map->size--;
            return;
        }
		prevLink = currLink;
        currLink = currLink->next;
    }
}

/**
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap* map, const char* key)
{
    int hashIndex = HASH_FUNCTION(key) % map->capacity;
    HashLink* currLink = map->table[hashIndex];
    while(currLink != 0){
        if(!strcmp(currLink->key, key)){
            return 1;
        }
        currLink = currLink->next;
    }

    return 0;
}

/**
 * Returns the number of links in the table.
 * @param map
 * @return Number of links in the table.
 */
int hashMapSize(HashMap* map)
{
    return map->size;
}

/**
 * Returns the number of buckets in the table.
 * @param map
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap* map)
{
    return map->capacity;
}

/**
 * Returns the number of table buckets without any links.
 * @param map
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap* map)
{
    int bucketCount = 0;

    for(int i = 0; i < map->capacity; i++){
        if(map->table[i] == NULL){
            bucketCount++;
        }
    }

    return bucketCount;
}

/**
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map
 * @return Table load.
 */
float hashMapTableLoad(HashMap* map)
{
    float loadFactor = ((float)hashMapSize(map))/hashMapCapacity(map);
    return loadFactor;
}

/**
 * Prints all the links in each of the buckets in the table.
 * @param map
 */
void hashMapPrint(HashMap* map)
{
    printf("\n----Hash Map Contents----\n");
    for(int i = 0; i < map->capacity; i++){
        HashLink* currLink = map->table[i];
        if(currLink != NULL){
            printf("Index %d: ", i);
            while(currLink != NULL){
            printf("[%s:%d] ", currLink->key, currLink->value);
            currLink = currLink->next;
            }
            printf("\n");
        }
    }
}
