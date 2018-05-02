/* Written by Hunter Bennett
 * Email: hkbennett2@crimson.ua.edu
 * ID: 11503255
 * Version: 2018.04.18
 * Assignment 7
 * Description: Prompts the user for information about the
 *  cache to be simulated, then calculates the values for
 *  each memory address and the final status of the cache.
 */

/* Typedefs */
typedef struct Block_* Block;
typedef struct Cache_* Cache;
typedef struct Memory_* Memory;

/* cacheCreate
 *
 * Function to create a new cache struct. Returns the new struct on success
 * and NULL on failure.
 *
 * @param	cache_size		Size of cache in bytes
 * @param	block_size		Size of each block in bytes
 * @param	rep_policy		1 = LRU, 2 = FIFO
 *
 * @return	success			cache
 * @return	failure			NULL
 */
 
Cache cacheCreate(int cache_size, int block_size, int rep_policy, int addr_count);

/* cacheDestroy
 * 
 * Function that destroys a created cache. Frees all allocated memory. If 
 * you pass in NULL, nothing happens. So make sure to set your cache = NULL
 * after you destroy it to prevent a double free.
 *
 * @param	cache			Target cache object to be destroyed
 *
 * @return	void
 */
 
void cacheDestroy(Cache cache);

/* cacheRead
 *
 * Function that reads data from a cache. Returns 0 on failure
 * or 1 on success. 
 *
 * @param	cache			Target cache struct
 * @param	address			Binary address
 *
 * @return	hit				1
 * @return	miss			0
 */

int cacheRead(Cache cache, char *address);

/* cacheWrite
 *
 * Function that writes data to the cache. Returns 0 on failure or
 * 1 on success. Frees any old tags that already existed in the
 * target slot.
 *
 * @param	cache			Target cache struct
 * @param	address			Binary address
 *
 * @return	hit				1
 * @return	miss			0
 */

int cacheWrite(Cache cache, char *address);

/* cachePrint
 *
 * Prints out the values of each slot in the cache
 * and its dirty bit, valid bit, tag, and data.
 *
 * @param	cache			Target cache struct
 *
 * @return	void
 */

void cachePrint(Cache cache);

/* getUserInt
 *
 * Prompts user for input within valid values.
 *
 * @param	prompt			Prompt to display to user
 * @param	max_val			Max value for valid input
 * @param	min_val			Min value for valid input
 *
 * @return	int				User-inputted integer
 */

int getUserInt(char *prompt, int max_val, int min_val);

/* inputPrint
 *
 * Prints out the values obtained from user input.
 *
 * @param	cache_size		Size of cache in bytes
 * @param	block_size		Size of each block in bytes
 * @param	nSA				Set-Associativity
 * @param	rep_policy		1 = LRU, 2 = FIFO
 * @param	filename		Name of input file
 *
 * @return	void
 */

void inputPrint(int mm_size, int cache_size, int block_size, int nSA, int rep_policy, char *filename);

/* memoryPrint
 *
 * Prints out the memory addresses used and their
 * respective mm block #, cache set #, cache block #s,
 * and hit status.
 *
 * @param	cache			Target cache struct
 *
 * @return	void
 */

void memoryPrint(Cache cache);

/* END OF FILE */
