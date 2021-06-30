#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef void* hash_table_t;

/**
 * @brief creates a hash map
 * @param[in] key_size size in byte of the key type
 * @param[in] value_size size in byte of the value type
 * @return the hash table object
 */
hash_table_t __hash_table_create(size_t key_size, size_t value_size);

/**
 * @brief destroys a hash table
 * @param[in] hash_table hash table to destroy
 */
void hash_table_destroy(hash_table_t hash_table);

/**
 * @brief insters a pair in the table
 * @param[in] hash_table the hash table to populate
 * @param[in] key_ptr pointer to the key
 * @param[in] value_ptr pointer to the value
 */
void __hash_table_insert(hash_table_t hash_table, void* key_ptr, void* value_ptr);

/**
 * @brief returns a pointer to the mapped value of the element with a key
 * @param[in] hash_table the hash table to fetch
 * @param[in] key_ptr pointer to the key
 * @return address of the value
 */
void* __hash_table_at(hash_table_t hash_table, void* key_ptr);

/**
 * @brief removes an entry from the table
 * @param[in] hash_table the hash table to fetch
 * @param[in] key_ptr pointer to the key
 */
void __hash_table_erase(hash_table_t hash_table, void* key_ptr);

/**
 * @brief returns the number of elements in the table
 * @param[in] hash_table hash table object
 * @return number of elements
 */
size_t hash_table_size(hash_table_t hash_table);

/**
 * @brief check if the hash table is empty
 * @param[in] queue hash table to check
 * @return emptiness
 */
bool hash_table_empty(hash_table_t hash_table);

#define hash_table_create(key_type, value_type) __hash_table_create(sizeof(key_type), sizeof(value_type))
#define hash_table_at(hash_table, key, value_ptr) { typeof(key) __key = key; value_ptr = __hash_table_at(hash_table, &__key); }
#define hash_table_insert(hash_table, key, value) { typeof(key) __key = key; typeof(value) __value = value; __hash_table_insert(hash_table, &__key, &__value); }
#define hash_table_erase(hash_table, key) { typeof(key) __key = key; __hash_table_erase(hash_table, &__key); }
