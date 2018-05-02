/* Written by Hunter Bennett
 * Email: hkbennett2@crimson.ua.edu
 * ID: 11503255
 * Version: 2018.04.18
 * Assignment 7
 * Description: Prompts the user for information about the
 *  cache to be simulated, then calculates the values for
 *  each memory address and the final status of the cache.
 */

/* Libraries */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cache_sim.h"

/* Structs */

/* Block
 *
 * Holds an integer that states if the bit is dirty or not,
 * an integer that states the validity of the bit, the tag
 * being held, and the data being held.
 *
 * @param dirty				0 = Clean, 1 = Dirty
 * @param valid				0 = Invalid, 1 = Valid
 * @param tag				Tag being held
 * @param data				Main memory block referenced
 */

struct Block_ {
	int access_count;
	int time_count;
	int dirty;
	int valid;
	char *tag;
	int data;
};

/* Cache
 *
 * Cache object that holds all the data about cache access as well as 
 * the write policy, sizes, and an array of blocks.
 *
 * @param	hits			# of cache accesses that hit valid data
 * @param	misses			# of cache accesses that missed valid data
 * @param	reads			# of reads from main memory
 * @param	writes			# of writes from main memory
 * @param	cache_size		Total size of the cache in bytes
 * @param	block_size		How big each block of data should be
 * @param	block_count		Total number of blocks
 * @param	rep_policy		1 = LRU, 2 = FIFO
 * @param	blocks			The actual array of blocks  
 */

struct Cache_ {
	
	int hits;
	int misses;
	int reads;
	int writes;

	int mm_size;
	int cache_size;
	int block_size;
	int block_count;
	int set_count;
	int nSA;
	int rep_policy;
	int addr_count;
	int addr_size[5];
	Block *block;
	Memory *memory;
};

struct Memory_ {
	char mode;
	int address;
	int cache_block_max;
	int cache_block_min;
	int cache_set;
	int mm_block;
	int hit;
};

/* btoi
 *
 * Converts a binary string to an integer. Returns 0 on error.
 *
 * @param	bin				Binary string to convert
 *
 * @result	dec				Decimal integer
 */

int btoi(char *bin) {
	int i, j, m, n;
    int len, dec;

	dec = 0;
	len = strlen(bin) - 1;

	for(i = 0; i <= len; i++) {
		n = (bin[i] - '0'); 
        if((n > 1) || (n < 0)) {
			return 0;
		}
        for(j = 1, m = len; m > i; m--) {
			j *= 2;
		}
		dec = dec + n * j;
	}

	return(dec);
}

/* itob
 *
 * Converts an unsigned integer into a string containing its
 * 32 bit long binary representation.
 *
 * @param	dec				Decimal integer to convert
 *
 * @result	bin				Binary string
 */

char *itob(unsigned int dec) {
	char *bin;
	int i;

	bin = (char *) malloc(sizeof(char) * 33);
	assert(bin != NULL);
	bin[32] = '\0';

	for(i = 0; i < 32; i++) {
        bin[32 - 1 - i] = (dec == ((1 << i) | dec)) ? '1' : '0';
    }

    return(bin);
}

/* ceil_log2
 *
 * Calculates the ceiling of log_2 of a number.
 *
 * @param	x				Unsigned integer to be used
 *
 * @result	y				Result of ceil(log_2)
 */

int ceil_log2(unsigned long long x) {
	int i, y;
	int j = 32;
	static const unsigned long long t[6] = {
		0xFFFFFFFF00000000ull,
		0x00000000FFFF0000ull,
		0x000000000000FF00ull,
		0x00000000000000F0ull,
		0x000000000000000Cull,
		0x0000000000000002ull
	};

	y = (((x & (x - 1)) == 0) ? 0 : 1);

	for(i = 0; i < 6; i++) {
		int k = (((x & t[i]) == 0) ? 0 : j);
		y += k;
		x >>= k;
		j >>= 1;
	}

	return(y);
}
 
int main(int argc, char **argv) {
	int goAgain = 1;

	do {
	Cache cache;
	FILE *file;

	int addr_count, i, j, k, n, valid;
	int mm_size = 0, cache_size = 0, block_size = 0, nSA = 0, rep_policy = 0;
	int hits = 0;
	double rate;
	char addr[64], input[128], line[128];
	char *filename = "N/A";

	/* Validate Inputs */
	inputPrint(mm_size, cache_size, block_size, nSA, rep_policy, filename);
	mm_size = getUserInt("\nEnter the size of the main memory in bytes: ", 32000, 4);
	inputPrint(mm_size, cache_size, block_size, nSA, rep_policy, filename);
	cache_size = getUserInt("\nEnter the size of the cache in bytes: ", mm_size, 2);
	inputPrint(mm_size, cache_size, block_size, nSA, rep_policy, filename);
	block_size = getUserInt("\nEnter the cache block/line size: ", cache_size, 2);
	inputPrint(mm_size, cache_size, block_size, nSA, rep_policy, filename);
	nSA = getUserInt("\nEnter the degrees of set-associativity: ", block_size, 1);
	inputPrint(mm_size, cache_size, block_size, nSA, rep_policy, filename);
	do {
		printf("\nEnter the replacement policy (L/F): ");
		fgets(input, sizeof(input), stdin);
		for(i = 0; i < sizeof(input); i++) {
			if(input[i] == '\n') {
				input[i] = '\0';
			}
		}
		valid = 0;
		if(input[0] == 'L') {
			rep_policy = 1;
			valid = 1;
		}
		else if(input[0] == 'F') {
			rep_policy = 2;
			valid = 1;
		}
	} while(!valid);
	inputPrint(mm_size, cache_size, block_size, nSA, rep_policy, filename);
	do {
		printf("\nEnter the name of the input file: ");
		fgets(input, sizeof(input), stdin);
		for(i = 0; i < sizeof(input); i++) {
			if(input[i] == '\n') {
				input[i] = '\0';
			}
		}
		valid = 1;
		filename = input;
		file = fopen(filename, "r");
		if(file == NULL) {
			printf("\nError: Could not open file %s", filename);
			valid = 0;
		}
	} while(!valid);

	/* Print out final input status */
	inputPrint(mm_size, cache_size, block_size, nSA, rep_policy, filename);

	/* Open the file for reading */
	fgets(line, sizeof(line), file);
	for(i = 0; i < sizeof(input); i++) {
		if(line[i] == '\n') {
			line[i] = '\0';
		}
	}
	addr_count = atoi(line);

	/* Skip a line */
	fgets(line, sizeof(line), file);

	cache = cacheCreate(cache_size, block_size, rep_policy, addr_count);

	cache->mm_size = mm_size;
	cache->nSA = nSA;
	cache->rep_policy = rep_policy;
	cache->set_count = cache->block_count / cache->nSA;

	cache->addr_size[0] = ceil_log2(cache->mm_size);
	cache->addr_size[1] = ceil_log2(cache->block_size);
	cache->addr_size[2] = ceil_log2((cache->cache_size / cache->block_size) / cache->nSA);
	cache->addr_size[3] = ceil_log2(cache->mm_size) - cache->addr_size[1] - cache->addr_size[2];
	n = cache->cache_size + cache->addr_size[3] + 1 + 1;

	printf("\nSimulator Output:");
	printf("\nTotal address lines required = %d", cache->addr_size[0]);
	printf("\nNumber of bits for offset = %d", cache->addr_size[1]);
	printf("\nNumber of bits for index = %d", cache->addr_size[2]);
	printf("\nNumber of bits for tag = %d", cache->addr_size[3]);
	printf("\nTotal cache size required = %d", n);

	k = 0;
	while(fgets(line, sizeof(line), file) != NULL) {
		i = 0, j = 0;
		cache->memory[k]->mode = line[i];
		while(line[i] != ' ') {
			i++;
		}
		while(line[i] != '\0') {
			addr[j] = line[i];
			i++;
			j++;
		}
		addr[j-1] = '\0';
		cache->memory[k]->address = atoi(addr);
		k++;
	}

	for(i = 0; i < cache->addr_count; i++) {
		cache->memory[i]->mm_block = cache->memory[i]->address / block_size;
		cache->memory[i]->cache_set = cache->memory[i]->mm_block % (cache->block_count / cache->nSA);
		cache->memory[i]->cache_block_min = cache->memory[i]->cache_set * cache->nSA;
		cache->memory[i]->cache_block_max = cache->memory[i]->cache_block_min + cache->nSA - 1;
		if(cache->memory[i]->mode == 'R') {
			cache->memory[i]->hit = cacheRead(cache, itob(cache->memory[i]->address));
		}
		else if(cache->memory[i]->mode == 'W') {
			cache->memory[i]->hit = cacheWrite(cache, itob(cache->memory[i]->address));
		}
		else {
			cache->memory[i]->hit = 0;
		}
	}

	for(i = cache->addr_count - 1; i > 0; i--) {
		for(j = 0; j < i; j++) {
			if(cache->memory[i]->mm_block == cache->memory[j]->mm_block) {
				hits++;
				j = i;
			}
		}
	}

	memoryPrint(cache);

	rate = ((double)hits / (double)cache->addr_count) * 100;
	printf("\n\nHighest possible hit rate = %d/%d = %f%%", hits, cache->addr_count, rate);
	rate = ((double)cache->hits / (double)cache->addr_count) * 100;
	printf("\nActual hit rate = %d/%d = %f%%", cache->hits, cache->addr_count, rate);

	printf("\n\nFinal status of the cache:");

	cachePrint(cache);

	/* Close the file and destroy the cache. */
	fclose(file);
	cacheDestroy(cache);
	cache = NULL;

	do {
		printf("\nContinue? (y/n): ");
		fgets(input, sizeof(input), stdin);
		for(i = 0; i < sizeof(input); i++) {
			if(input[i] == '\n') {
				input[i] = '\0';
			}
		}
		valid = 0;
		if(input[0] == 'y') {
			goAgain = 1;
			valid = 1;
		}
		else if(input[0] == 'n') {
			goAgain = 0;
			valid = 1;
		}
	} while(!valid);

} while(goAgain);

	system("clear");
	printf("\n\n\n\nHave a nice day :)\n\n\n\n");

	return(1);
}

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

Cache cacheCreate(int cache_size, int block_size, int rep_policy, int addr_count) {
	Cache cache;
	int i;

	cache = (Cache) malloc(sizeof(struct Cache_));
	if(cache == NULL) {
		fprintf(stderr, "\nCould not allocate memory for cache.");
		return NULL;
	}

	cache->hits = 0;
	cache->misses = 0;
	cache->reads = 0;
	cache->writes = 0;

	cache->cache_size = cache_size;
	cache->block_size = block_size;
	cache->rep_policy = rep_policy;
	cache->addr_count = addr_count;

	/* Calculate block_count */
	cache->block_count = cache_size / block_size;

	cache->block = (Block*) malloc(sizeof(Block) * cache->block_count);
	assert(cache->block != NULL);

	cache->memory = (Memory*) malloc(sizeof(Memory) * cache->addr_count);
	assert(cache->memory != NULL);

	/* Insert blocks where valid = 0 */
	for(i = 0; i < cache->block_count; i++) {
		cache->block[i] = (Block) malloc(sizeof(struct Block_));
		assert(cache->block[i] != NULL);
		cache->block[i]->access_count = 0;
		cache->block[i]->time_count = 0;
		cache->block[i]->valid = 0;
		cache->block[i]->dirty = -1;
		cache->block[i]->tag = NULL;
		cache->block[i]->data = -1;
	}

	for(i = 0; i < cache->addr_count; i++) {
		cache->memory[i] = (Memory) malloc(sizeof(struct Memory_));
		assert(cache->memory[i] != NULL);

		cache->memory[i]->mode = 'R';
		cache->memory[i]->address = 0;
		cache->memory[i]->cache_block_max = 0;
		cache->memory[i]->cache_block_min = 0;
		cache->memory[i]->cache_set = 0;
		cache->memory[i]->mm_block = 0;
		cache->memory[i]->hit = 0;
	}

	return(cache);
}

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

void cacheDestroy(Cache cache) {
	int i;

	if(cache != NULL) {
		for(i = 0; i < cache->block_count; i++) {
			if(cache->block[i]->tag != NULL) {
				free(cache->block[i]->tag);
			}
			free(cache->block[i]);
		}
		free(cache->block);
		free(cache->memory);
		free(cache);
	}

	return;
}

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

int cacheRead(Cache cache, char *address) {
	char *tag, *index, *offset, *data;
	int i, j;
	int hit = 0;
	int time = -1;
	Block block;

	/* Validate inputs */
	if(cache == NULL) {
		fprintf(stderr, "\nError: Must supply a valid cache to write to.");
		return(0);
	}
	if(address == NULL) {
		fprintf(stderr, "\nError: Must supply a valid memory address.");
		return(0);
	}

	/* Get tag, index, and offset */
	i = strlen(address) - cache->addr_size[0];

	tag = (char *) malloc(sizeof(char) * (cache->addr_size[3] + 2));
	assert(tag != NULL);
	tag[cache->addr_size[3]] = '\0';
	for(j = 0; j < cache->addr_size[3]; j++) {
		tag[j] = address[i];
		i++;
	}

	index = (char *) malloc(sizeof(char) * (cache->addr_size[2] + 2));
	assert(index != NULL);
	index[cache->addr_size[2]] = '\0';
	for(j = 0; j < cache->addr_size[2]; j++) {
		index[j] = address[i];
		i++;
	}

	offset = (char *) malloc(sizeof(char) * (cache->addr_size[1] + 2));
	assert(offset != NULL);
	offset[cache->addr_size[1]] = '\0';
	for(j = 0; j < cache->addr_size[1]; j++) {
		offset[j] = address[i];
		i++;
	}

	/* Find cache block for reading */
	for(i = 0; i < cache->block_count; i++) {
		if((i >= btoi(index) * cache->nSA)&&(i < (btoi(index) + 1) * cache->nSA)) {
			if(cache->block[i]->valid == 0) {
				block = cache->block[i];
				block->tag = tag;
				i = cache->block_count;
			}
			else if(cache->rep_policy == 1) {
				if(cache->block[i]->time_count > time) {
					block = cache->block[i];
					time = cache->block[i]->time_count;
				}
			}
			else if(cache->rep_policy == 2) {
				if(cache->block[i]->time_count > time) {
					block = cache->block[i];
					time = cache->block[i]->time_count;
				}
			}
		}
	}

	data = (char *) malloc(sizeof(char) * (cache->addr_size[2] + cache->addr_size[3] + 1));
	j = 0;
	for(i = 0; i < cache->addr_size[3]; i++) {
		data[j] = tag[i];
		j++;
	}
	for(i = 0; i < cache->addr_size[2]; i++) {
		data[j] = index[i];
		j++;
	}

	block->data = btoi(data);
	printf("\n%s", tag);
	if(block->valid == 1 && strcmp(block->tag, tag) == 0) {
		hit = 1;
		cache->hits++;
	}
	else {
		cache->misses++;
		cache->reads++;
	}
	block->tag = tag;
	block->dirty = 0;
	block->valid = 1;

	for(i = 0; i < cache->block_count; i++) {
		cache->block[i]->time_count++;
	}
	block->time_count = 0;

	free(offset);
	free(index);
	free(data);

	return(hit);
}

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

int cacheWrite(Cache cache, char *address) {
	char *tag, *index, *offset, *data;
	int i, j;
	int hit = 0;
	int time = -1;
	Block block;
	/* Validate inputs */
	if(cache == NULL) {
		fprintf(stderr, "\nError: Must supply a valid cache to write to.");
		return(0);
	}
	if(address == NULL) {
		fprintf(stderr, "\nError: Must supply a valid memory address.");
		return(0);
	}

	/* Get tag, index, and offset */
	i = strlen(address) - cache->addr_size[0];

	tag = (char *) malloc(sizeof(char) * (cache->addr_size[3] + 2));
	assert(tag != NULL);
	tag[cache->addr_size[3]] = '\0';
	for(j = 0; j < cache->addr_size[3]; j++) {
		tag[j] = address[i];
		i++;
	}

	index = (char *) malloc(sizeof(char) * (cache->addr_size[2] + 2));
	assert(index != NULL);
	index[cache->addr_size[2]] = '\0';
	for(j = 0; j < cache->addr_size[2]; j++) {
		index[j] = address[i];
		i++;
	}

	offset = (char *) malloc(sizeof(char) * (cache->addr_size[1] + 2));
	assert(offset != NULL);
	offset[cache->addr_size[1]] = '\0';
	for(j = 0; j < cache->addr_size[1]; j++) {
		offset[j] = address[i];
		i++;
	}

	/* Find cache block for writing */
	for(i = 0; i < cache->block_count; i++) {
		if((i >= btoi(index) * cache->nSA)&&(i < (btoi(index) + 1) * cache->nSA)) {
			if(cache->block[i]->valid == 0) {
				block = cache->block[i];
				block->tag = tag;
				i = cache->block_count;
			}
			else if(cache->rep_policy == 1) {
				if(cache->block[i]->time_count > time) {
					block = cache->block[i];
					time = cache->block[i]->time_count;
				}
			}
			else if(cache->rep_policy == 2) {
				if(cache->block[i]->time_count > time) {
					block = cache->block[i];
					time = cache->block[i]->time_count;
				}
			}
		}
	}

	data = (char *) malloc(sizeof(char) * (cache->addr_size[2] + cache->addr_size[3] + 1));
	j = 0;
	for(i = 0; i < cache->addr_size[3]; i++) {
		data[j] = tag[i];
		j++;
	}
	for(i = 0; i < cache->addr_size[2]; i++) {
		data[j] = index[i];
		j++;
	}

	block->data = btoi(data);
	printf("\n%s", tag);
	if(block->valid == 1 && strcmp(block->tag, tag) == 0) {
		hit = 1;
		cache->hits++;
	}
	else {
		cache->misses++;
		cache->reads++;
		if(block->dirty == 1) {
			cache->writes++;
		}
	}
	block->tag = tag;
	block->dirty = 1;
	block->valid = 1;

	for(i = 0; i < cache->block_count; i++) {
		cache->block[i]->time_count++;
	}
	block->time_count = 0;

	free(offset);
	free(index);
	free(data);

	return(hit);
}

/* cachePrint
 *
 * Prints out the values of each slot in the cache
 * and its dirty bit, valid bit, tag, and data.
 *
 * @param	cache			Target cache struct
 *
 * @return	void
 */

void cachePrint(Cache cache) {
	int i, j;
	//char tag[cache->addr_size[3]];

	printf("\n Cache blk #\tdirty bit\tvalid bit\ttag\tData");
	printf("\n****************************************************************");
	for(i = 0; i < cache->block_count; i++) {
		printf("\n %d", i);
		if(cache->block[i]->dirty == -1) {
			printf("\t\tx");
		}
		else {
			printf("\t\t%d", cache->block[i]->dirty);
		}
		printf("\t\t%d", cache->block[i]->valid);
		printf("\t\t");
		if(cache->block[i]->tag == NULL) {
			for(j = 0; j < cache->addr_size[3]; j++) {
				printf("x");
			}
		}
		else {
			printf("%s", cache->block[i]->tag);
		}
		if(cache->block[i]->data == -1) {
			printf("\tX");
		}
		else {
			printf("\tmm blk #%d", cache->block[i]->data);
		}
	}
}

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

int getUserInt(char *prompt, int max_val, int min_val) {
	char input[128];
	int input_int, valid;

	do {
		printf(prompt);
		fgets(input, sizeof(input), stdin);
		for(int i = 0; i < sizeof(input); i++) {
			if(input[i] == '\n') {
				input[i] = '\0';
			}
		}
		valid = 1;
		input_int = atoi(input);
		if((input_int < min_val)||(input_int > max_val)) {
			printf("Error: Value must be between %d and %d", min_val, max_val);
			valid = 0;
		}
	} while(!valid);

	return(input_int);
}

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

void inputPrint(int mm_size, int cache_size, int block_size, int nSA, int rep_policy, char *filename) {
	system("clear");

	printf("\nMain Memory Size (bytes): ");
	if(mm_size)
		printf("%d", mm_size);
	else
		printf("N/A");

	printf("\nCache Memory Size (bytes): ");
	if(cache_size)
		printf("%d", cache_size);
	else
		printf("N/A");

	printf("\nCache Block/Line Size: ");
	if(block_size)
		printf("%d", block_size);
	else
		printf("N/A");

	printf("\n\nDegree of Set Associativity: ");
	if(nSA)
		printf("%d", nSA);
	else
		printf("N/A");

	printf("\nReplacement Policy: ");
	if(rep_policy == 1)
		printf("LRU");
	else if(rep_policy == 2)
		printf("FIFO");
	else
		printf("N/A");
	printf("\nInput File: %s\n", filename);

}

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

void memoryPrint(Cache cache) {
	int i;
	Memory memory;

	printf("\n\n mm address\tmm blk #\tcm set #\tcm blk #\thit/miss");
	printf("\n************************************************************************");
	for(i = 0; i < cache->addr_count; i++) {
		memory = cache->memory[i];
		printf("\n %d\t", memory->address);
		printf("\t%d", memory->mm_block);
		printf("\t\t%d", memory->cache_set);
		if(cache->nSA > 1) {
			printf("\t\t%d - %d", memory->cache_block_min, memory->cache_block_max);
		}
		else {
			printf("\t\t%d", memory->cache_block_min);
		}
		if(memory->hit) {
			printf("\t\thit");
		}
		else {
			printf("\t\tmiss");
		}
	}
}

/* END OF FILE */
