#include <stdint.h>

typedef struct {
	char key[KEY_SIZE];
	size_t data_size;
	intptr_t data_page;
	uint64_t access_time;
	time_t timeout;
	intptr_t edges;
	intptr_t lru_prev;
	intptr_t lru_next;
	intptr_t heap_entry;
} node;

typedef struct {
	uint64_t id;
	intptr_t target_edge;
	uint64_t target_id;
	intptr_t next;
} edge;

typedef struct {
	intptr_t next;
	char data[PAGE_DATA_SIZE];
} page;

typedef struct {
	time_t timeout;
	intptr_t node;
} heap;

typedef struct {
	intptr_t hash_size;
	intptr_t pages_size;
	intptr_t edges_size;
	intptr_t heap_size; /* = hash_size*hash_depth */
	node (*hash)[HASH_DEPTH];
	heap  *timeoutheap;
	edge *edges;
	page *pages;
} cache;


static hash *get_hash_entry(cache *c,char const *key)
{
	intptr_t H=calc_hash(key,c);
	int i;
	hash *hash_entry=c->hash[H];
	for(i=0;i<HASH_DEPTH;i++,hash_entry++) {
		if(strcmp(hash_entry->key,key)==0) {
			return hash_entry;
		}
	}
	return NULL;
}

static int walk_and_validate(cache *c,hash *entry)
{
	queue *queue=NULL;
	intptr_t limit=c->hash_size*HASH_DEPTH;
	intptr_t count=1;
	do {
		edge *e;
		
	} while(!queue_is_empty(queue));
}

static int fetch(cache *c,char const *key,void **out_buffer,size_t *out_size)
{
	hash *entry=get_hash_entry(c,key);
	if(!entry)
		return 0;
	time_t now;
	time(&now);
	if(entry->timeout<now) {
		return 0;
	}
	int res;
	if(walk_and_validate(c,entry)!=1) {
		return 0;
	}
	update_lru(c,entry);
	copy_pages(c,entry,out_buffer,out_size);
	return 1;
}

static int insert(cache *c,char const *key,void const *data,size_t size,time_t timeout)
{
	drop_key(c,key);
	hash *entry=create_new_entry();
	
}
