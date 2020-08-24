#include <unistd.h>
#include <stdio.h>
#include "smalloc.h" 

sm_container_t sm_head = {
	0,
	&sm_head, 
	&sm_head,
	0 
} ;

static 
void * 
_data (sm_container_ptr e)
{
	return ((void *) e) + sizeof(sm_container_t) ;
}

static 
void 
sm_container_split (sm_container_ptr hole, size_t size)
{
	sm_container_ptr remainder = (sm_container_ptr) (_data(hole) + size) ;
	remainder->dsize = hole->dsize - size - sizeof(sm_container_t) ;
	remainder->status = Unused ;
	remainder->next = hole->next ;
	remainder->prev = hole ;
	hole->dsize = size ;
	hole->next->prev = remainder ;
	hole->next = remainder ;
}

static 
void * 
retain_more_memory (int size)
{
	sm_container_ptr hole ;
	int pagesize = getpagesize() ;
	int n_pages = 0 ;

	n_pages = (sizeof(sm_container_t) + size + sizeof(sm_container_t)) / pagesize  + 1 ;
	hole = (sm_container_ptr) sbrk(n_pages * pagesize) ;
	if (hole == 0x0)
		return 0x0 ;
	
	hole->status = Unused ;
	hole->dsize = n_pages * getpagesize() - sizeof(sm_container_t) ;
	return hole ;
}

void * 
smalloc (size_t size) 
{
	sm_container_ptr hole = 0x0, itr = 0x0 ;
    size_t error;
	int init = 0;
    
	for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
		if (itr->status == Busy)
			continue ;
		if ((itr->dsize == size) || (size < itr->dsize)) {
			if(init == 0){
				hole = itr;
				error = itr->dsize - size;
				init = 1;
			}
        	else if(error > itr->dsize - size){
			    hole = itr ;
				error = itr->dsize - size;
            }
		}
	}

	if (hole == 0x0) {
		hole = retain_more_memory(size) ;
		if (hole == 0x0)
			return 0x0 ;
		hole->next = &sm_head ;
		hole->prev = sm_head.prev ;
		(sm_head.prev)->next = hole ;
		sm_head.prev = hole ;
	}
	if (size < hole->dsize) 
		sm_container_split(hole, size) ;
	hole->status = Busy ;
	return _data(hole) ;
}

void * srealloc (void * p, size_t newsize){
	sm_container_ptr itr = 0x0, previous = 0x0 ;
	sm_container_ptr P = (sm_container_ptr)p;

	for (itr = sm_head.next; itr != &sm_head ; itr = itr->next) {
		if(_data(itr) == P){
			previous = itr;
			break;
		}
	}
	
	if(previous->next->status == Unused){	
		if(newsize == previous->dsize + previous->next->dsize + sizeof(sm_container_t)){
			previous->dsize = previous->dsize + previous->next->dsize + sizeof(sm_container_t);
			previous->next = previous->next->next;
			previous->next->prev = previous;
			return _data(previous);
		}
		else if(newsize < previous->dsize + previous->next->dsize + sizeof(sm_container_t)){

			previous->dsize = previous->dsize + previous->next->dsize + sizeof(sm_container_t);
			previous->next = previous->next->next;
			previous->next->prev = previous;

			sm_container_split(previous, newsize);
			if(previous->next->dsize == 0){
				previous->next->next->prev = previous;
				previous->next = previous->next->next;
			}
			return _data(previous);
		}
		else{
			sm_container_ptr new;
			new = smalloc(newsize);
			sfree(previous);
			return _data(new);
		}
	}
	else{
		sm_container_ptr new;
		new = smalloc(newsize);
		sfree(previous);
		return _data(new);
	}
}

void sshrink(){
	sm_container_ptr itr = 0x0, last = 0x0, current = 0x0 ;
	int pagesize = getpagesize() ;
	int n_pages = 0 ;
	last = sm_head.prev;

	if(last->status == Unused){
		n_pages = last->dsize / pagesize;
		size_t deleted = n_pages * pagesize;
		size_t remain = last->dsize - deleted;
		if(deleted == (last->dsize + sizeof(sm_container_t))){
			last->next->prev = last->prev;
			last->prev->next = last->next;
		}
		else{
			last->dsize -= deleted;
			if(last->dsize == 0){
				last->prev->next = last->next;
				last->next->prev = last->prev;
			}
		}
		if(n_pages > 0){
			brk(last + remain);
		}
	}
}

void 
sfree (void * p)
{
	sm_container_ptr itr ;
	for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
		if (p == _data(itr)) {
			itr->status = Unused ; //여기에다가 merge를 해줘야함!
			if(itr->prev->status == Unused){
				//merge
				itr->prev->next = itr->next;
				itr->next->prev = itr->prev;
				itr->prev->dsize = itr->prev->dsize + itr->dsize + sizeof(sm_container_t);
				itr = itr->prev;
			}
			if(itr->next->status == Unused){
				itr->next->prev = itr->prev;
				itr->prev->next = itr->next;
				itr->next->dsize = itr->next->dsize + itr->dsize + sizeof(sm_container_t);
				itr = itr->next;
			}
			break ;
		}
	}
}

void 
print_sm_containers ()
{
	sm_container_ptr itr ;
	int i ;

	printf("==================== sm_containers ====================\n") ;
	for (itr = sm_head.next, i = 0 ; itr != &sm_head ; itr = itr->next, i++) {
		printf("%3d:%p:%s:", i, _data(itr), itr->status == Unused ? "Unused" : "  Busy") ;
		printf("%8d:", (int) itr->dsize) ;

		int j ;
		char * s = (char *) _data(itr) ;
		for (j = 0 ; j < (itr->dsize >= 8 ? 8 : itr->dsize) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

}

void print_mem_uses(){
	sm_container_ptr itr ;
	int i ;
	size_t retained = 0;
	size_t allocated = 0;
	size_t not_allocated = 0;

	for (itr = sm_head.next, i = 0 ; itr != &sm_head ; itr = itr->next, i++) {
		if(itr->status == Busy) allocated += itr->dsize;
		else not_allocated += itr->dsize;
		//printf("itr dsize : %lu, sizeof(sm_container_t) : %lu\n",itr->dsize, sizeof(sm_container_t));
		retained += (itr->dsize + sizeof(sm_container_t));
	}

	printf("==================== sm_memory_uses ====================\n") ;
	printf("(1). retained memory 		: %lu\n", retained);
	printf("(2). allocated memory 		: %lu\n", allocated);
	printf("(3). retained but not allocated : %lu\n\n", not_allocated);
}