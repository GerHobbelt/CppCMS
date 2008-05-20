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
	intptr_t lru_first;
	intptr_t lru_last;
	intptr_t free_pages;
	intptr_t free_pages_count;
	intptr_t free_edges;
	intptr_t free_edges_count;
	intptr_t heap_size;
} control;

typedef struct {
	intptr_t hash_size;
	intptr_t pages_size;
	intptr_t edges_size;
	intptr_t heap_size; /* = hash_size*hash_depth */
	node (*hash)[HASH_DEPTH];
	heap  *timeoutheap;
	edge *edges;
	page *pages;
	control *control_data;
} cache;


static node *get_hash_entry(cache *c,char const *key)
{
	intptr_t H=calc_hash(c,key);
	int i;
	node *hash_entry=c->hash[H];
	for(i=0;i<HASH_DEPTH;i++,hash_entry++) {
		if(strcmp(hash_entry->key,key)==0) {
			return hash_entry;
		}
	}
	return NULL;
}

static int walk_and_validate(cache *c,node *entry)
{
	edge *walk_list=NULL;
	/*****************/
	int limit=c->maximal_depth;
	edge *walk_list=malloc(sizeof(edge*)*limit);
	if(!walk_list) goto error_exit;
	edge *p=c->edges+entry->edges;
	int rind=0,wind=0;
	int res=1;
	do {
		for(;p;p=c->edges+p->next) {
			if(p->target_id==0)
				continue;
			if(p->target_id != c->edges[p->target_edge]) {
				res=0;
				goto exit;
			}
			if(wind>=limit) {
				res=-1;
				goto exit;
			}
			walk_list[wind]=c->edges + p->target_edge;
			wind++;
		}
		if(rind<wind){
			p=walk_list[rind];
			rind++;
		}
	}while(p);
	res=1;
exit:
	free(walk_list);
	return res;
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

static void drop(cache *c,char const *key)
{
	node *entry=get_hash_entry(c,deps[i]);
	if(!entry)
		return;
	remove_from_lru(c,entry);
	remove_from_heap(c,entry);
	free_pages(c,c->edges + entry->pages);
	free_edges(c,c->pages + entry->edges);
	free_node(c,entry);
}

static pages_list *allocate_pages(cache *c,void const *data,int size)
{
	int N = size/PAGE_DATA_SIZE + ((size % PAGE_DATA_SIZE) ? 1 : 0);
	while(c->control_data->free_pages_count<N) {
		if(!remove_by_timeout() && !remove_by_oldest())
			return NULL;
	}
	pages_list *res=c->pages+c->control_data->free_pages;
	intptr_t free=c->control_data->free_pages;
	for(i=0;i<N;i++){
		if(i+1==N) {
			c->control_data->free_pages=c->pages[free].next;
			c->pages[free].next=0;
		}
		else
			free=c->pages[free].next;
	}
	free=res - c->pages;
	for(i=0;i<N;i++) {
		if(i+1<N) {
			memcpy(c->pages[free].data,data+i*PAGE_DATA_SIZE,PAGE_DATA_SIZE)
		}
		else {
			memcpy(c->pages[free].data,data+i*PAGE_DATA_SIZE,size % PAGE_DATA_SIZE)
		}
		free=c->pages[free].next;
	}
	c->control_data->free_pages_count-=N;
	return res;
}

static void free_pages(cache *c,page_list *lst)
{
	if(lst==NULL)
		return;
	intptr_t p=lst - c->pages;
	int N=0;
	intptr_t prev=p;
	while(p) {
		N++;
		prev=p;
		p=c->pages[p];
	}
	c->control_data->free_pages_count+=N;

}

static void insert(	cache *c,
			char const *key,
			char const **deps,
			int deps_size,
			void const *data,
			size_t size,
			time_t timeout,
			int manage_by_lru)
{
	intptr_t *targets=NULL;
	edge *edges_list=NULL;
	page *pages_list=NULL;
	node *new_node=NULL;
	/* End of cleanup declarations */
	drop_key(c,key);

	targets=malloc(sizeof(intptr_t)*deps_size);
	if(!targets) goto error_exit;

	time_t current;
	time(&current);

	for(i=0;i<deps_size;i++) {
		node *entry=get_hash_entry(c,deps[i]);
		if(entry->timeout<current)
			goto error_exit;
		if(walk_and_validate(c,entry)!=1)
			goto error_exit;
		targets[i]=entry->edges;
	}
	
	edge *edges_list=allocate_edges(c,deps+1);
	if(!edges_list) goto error_exit;
	page *pages_list=allocate_pages(c,data,size);
	if(!pages_list) goto error_exit;
	node *new_node=allocate_node(c,key);
	if(!new_node) goto error_exit;
	edge *tmp;
	for(tmp=edges_list,i=0;tmp;tmp=c->edges+tmp->next,i++) {
		if(i==0){
			tmp->id=get_id(c);
			tmp->target_id=-1;
			tmp->target_edge=0;
		}
		else {
			tmp->id=0;
			tmp->target_edge=targets[i-1];
			tmp->target_id=c->edges[targets[i-1]].id;
		}
	}

	strncpy(new_node->key,key);
	new_node->data_size=size;
	new_node->data_pages=pages_list - c->pages;
	new_node->access_time=get_atomic_time(c);
	new_node->timeout=timeout;
	new_node->edges=edges_list - c->edges;
	if(manage_by_lru){
		insert_to_lru(c,new_node);
	}
	else {
		new_node->lru_prev=new_node->lru_next=0;
	}
	insert_to_heap(c,new_node,timeout);
	free(targets);
	return;
error_exit:
	/* cleanup */
	free(targets);
	free_node(c,new_node);
	free_pages(c,pages_list);
	free_edges(c,edges_list);
}


