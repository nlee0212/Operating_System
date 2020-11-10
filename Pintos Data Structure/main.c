#include "bitmap.h"
#include "debug.h"
#include "hash.h"
#include "hex_dump.h"
#include "limits.h"
#include "list.h"
#include "round.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct hash_item{
	struct hash_elem elem;
	int data;
};

void square(struct hash_elem *e, void *aux);
void triple(struct hash_elem *e, void *aux);
bool list_less(const struct list_elem *a, const struct list_elem *b, void *aux);
int find_index_by_name(char**, char* name, int cnt);
void destructor(struct hash_elem *e, void *aux);
void hash_dump_all(struct hash_elem *e, void *aux);
unsigned hash_hash(const struct hash_elem *e, void *aux);
bool hash_less(const struct hash_elem *a, const struct hash_elem *b, void *aux);
int find_name(char* name,char**,char**,char**,int,int,int);

int main(void){
	char cmd[401]={'\0',};// save input command
	char* tok_cmd[7];// tokenize input command
	int size = 1; // word count  of the input command
	int i;

	struct bitmap* bitmap_list[10];//bitmap list
	char **bitmap_name;//bitmap name list
	int bitmap_cnt = 0;//bitmap count

	struct list list_list[10];
	char **list_name;
	int list_cnt = 0;

	struct hash hash_list[10];
	char **hash_name;
	int hash_cnt = 0;

	bitmap_name = (char**)malloc(sizeof(char*)*10);
	list_name = (char**)malloc(sizeof(char*)*10);
	hash_name = (char**)malloc(sizeof(char*)*10);

	for(i=0;i<10;i++){
		bitmap_name[i] = (char*)malloc(sizeof(char)*10);
		list_name[i] = (char*)malloc(sizeof(char)*10);
		hash_name[i] = (char*)malloc(sizeof(char)*10);
	}

	while(1){

	fgets(cmd,400,stdin);
	
	for(i=0;;i++){
		if(i==0) tok_cmd[i]=strtok(cmd," \n\t");
		else tok_cmd[i] = strtok(NULL," \n\t");

		//if end of command
		if(!tok_cmd[i]) break;
	}
	
	//save word count of input command
	size = i;

	//if first word of command == "create"
	if(!strcmp("create",tok_cmd[0])){
		//create bitmap bitmap_name bitmap_size
		if(!strcmp("bitmap",tok_cmd[1])){
			if(size!=4) continue;
			strcpy(bitmap_name[bitmap_cnt],tok_cmd[2]);
			bitmap_list[bitmap_cnt++] = bitmap_create(atoi(tok_cmd[3]));
		}

		else if(size == 3){
			//create list list_name
			if(!strcmp("list",tok_cmd[1])){
				strcpy(list_name[list_cnt],tok_cmd[2]);
				list_init(&list_list[list_cnt++]);
			}

			//create hashtable hashtable_name
			else if(!strcmp("hashtable",tok_cmd[1])){
				strcpy(hash_name[hash_cnt],tok_cmd[2]);
				hash_init(&hash_list[hash_cnt++],hash_hash,hash_less,NULL);
			}
		}

	}

	//if first word of command == "dumpdata"
	else if(!strcmp("dumpdata",tok_cmd[0]) && size==2){
		//prints the given data structure to standard out (STDOUT)s
		
		int type = find_name(tok_cmd[1],list_name,hash_name,bitmap_name,list_cnt,hash_cnt,bitmap_cnt);
		int index = type%10;
		type /= 10;

		//if in list
		//dumpdata list_name
		if(type == 1){
			if(!list_empty(&list_list[index])){
				struct list_elem *elem_cur;
				struct list_item *item_cur;
			
				for(elem_cur = list_begin(&list_list[index]);elem_cur != list_end(&list_list[index]);elem_cur = list_next(elem_cur)){
					item_cur = list_entry(elem_cur,struct list_item,elem);
					printf("%d ",item_cur->data);
				}

				printf("\n");
			}
		}

		//dumpdata bitmap_name
		else if(type == 2 && bitmap_list[index]!=NULL){
			struct bitmap *b = bitmap_list[index];

			for(int i=0;i<bitmap_size(b);i++){
				if(bitmap_test(b,i)) printf("1");
				else printf("0");
			}

			printf("\n");
		}

		//dumpdata hashtable_name
		else if(type == 3 && !hash_empty(&hash_list[index])){
			hash_apply(&hash_list[index],hash_dump_all);
			printf("\n");
		}

		else continue;
	}

	else if(!strcmp("delete",tok_cmd[0]) && size == 2){
		int type = find_name(tok_cmd[1],list_name,hash_name,bitmap_name,list_cnt,hash_cnt,bitmap_cnt);
		int index = type % 10;
		type /= 10;

		//if in list
		//delete list_name
		if(type == 1 && !list_empty(&list_list[index])){
			while(!list_empty(&list_list[index])){
				list_pop_front(&list_list[index]);
				//free(e);
			}
			
		}

		//delete bitmap_name
		//if in bitmap
		else if(type == 2 && bitmap_list[index]!=NULL){
			bitmap_destroy(bitmap_list[index]);

		}

		//delete hashtable_name
		//if in hash
		else if(type == 3 && !hash_empty(&hash_list[index])){
			hash_destroy(&hash_list[index],destructor);

		}

	}

	//quit
	else if(!strcmp("quit",tok_cmd[0]) && size == 1){
		break;
	}

/*
   =============================LIST COMMANDS=============================
*/

	//list_push_back list_name list_item_data
	else if(!strcmp("list_push_back",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		int input_data = atoi(tok_cmd[2]);

		struct list_elem *node = (struct list_elem*)malloc(sizeof(struct list_elem));
		struct list_item *item = list_entry(node,struct list_item,elem);
		item->data = input_data;

		list_push_back(&list_list[index],node);
	}

	//list_push_front list_name list_item_data
	else if(!strcmp("list_push_front",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		int input_data = atoi(tok_cmd[2]);

		struct list_elem *node = (struct list_elem*)malloc(sizeof(struct list_elem));
		struct list_item *item = list_entry(node,struct list_item,elem);
		item->data = input_data;

		list_push_front(&list_list[index],node);
	}

	//list_front list_name
	else if(!strcmp("list_front",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);

		struct list_elem *e = list_begin(&list_list[index]);
		struct list_item *item = list_entry(e,struct list_item,elem);

		printf("%d\n",item->data);
	}

	//list_back list_name
	else if(!strcmp("list_back",tok_cmd[0]) && size == 2){	
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		
		struct list_elem *e = list_rbegin(&list_list[index]);
		struct list_item *item = list_entry(e,struct list_item,elem);

		printf("%d\n",item->data);
	}

	//list_pop_back list_name
	else if(!strcmp("list_pop_back",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		
		list_pop_back(&list_list[index]);
	}

	//list_pop_front list_name
	else if(!strcmp("list_pop_front",tok_cmd[0]) && size == 2){	
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);

		list_pop_front(&list_list[index]);
	}

	//list_insert list_name list_index list_data
	else if(!strcmp("list_insert",tok_cmd[0]) && size == 4){
		int name_index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		int list_index = atoi(tok_cmd[2]);
		int list_data = atoi(tok_cmd[3]);

		struct list_elem *before = list_begin(&list_list[name_index]);
		struct list_elem *e = (struct list_elem*)malloc(sizeof(struct list_elem));
		struct list_item *e_item = list_entry(e,struct list_item,elem);
		e_item->data = list_data;

		for(int i=0;i<list_index;i++) before = list_next(before);

		list_insert(before,e);
	}

	//list_insert_ordered list_name list_data
	else if(!strcmp("list_insert_ordered",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		int list_data = atoi(tok_cmd[2]);

		struct list_elem *e = (struct list_elem*)malloc(sizeof(struct list_elem));
		struct list_item *e_item = list_entry(e,struct list_item,elem);
		e_item->data = list_data;

		list_insert_ordered(&list_list[index],e,list_less,NULL);
	}

	//list_empty list_name
	else if(!strcmp("list_empty",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);

		if(list_empty(&list_list[index])) printf("true\n");
		else printf("false\n");
	}

	//list_size list_name
	else if(!strcmp("list_size",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		printf("%zu\n",list_size(&list_list[index]));
	}

	//list_max list_name
	else if(!strcmp("list_max",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		struct list_elem *max = list_max(&list_list[index],list_less,NULL);
		struct list_item *max_item = list_entry(max,struct list_item,elem);

		printf("%d\n",max_item->data);
	}

	//list_min list_name
	else if(!strcmp("list_min",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		struct list_elem *min = list_min(&list_list[index],list_less,NULL);
		struct list_item *min_item = list_entry(min,struct list_item,elem);

		printf("%d\n",min_item->data);
	}

	//list_remove list_name list_index
	else if(!strcmp("list_remove",tok_cmd[0]) && size == 3){
		int name_index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		int list_index = atoi(tok_cmd[2]);

		struct list_elem *e = list_begin(&list_list[name_index]);

		for(int i=0;i<list_index;i++) e = list_next(e);

		list_remove(e);
	}

	//list_reverse list_name
	else if(!strcmp("list_reverse",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		
		list_reverse(&list_list[index]);
	}

	//list_shuffle list_name
	else if(!strcmp("list_shuffle",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		
		list_shuffle(&list_list[index]);
	}

	//list_sort list_name
	else if(!strcmp("list_sort",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);

		list_sort(&list_list[index],list_less,NULL);
	}

	//list_splice list_name_1 list_index_before list_name_2 list_index_first list_index_last(exclusive)
	else if(!strcmp("list_splice",tok_cmd[0]) && size == 6){
		int index_1 = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		int index_2 = find_index_by_name(list_name,tok_cmd[3],list_cnt);
		int index_before = atoi(tok_cmd[2]);
		int index_first = atoi(tok_cmd[4]);
		int index_last = atoi(tok_cmd[5]);

		int i;

		struct list_elem* before = list_begin(&list_list[index_1]);
		struct list_elem* first = list_begin(&list_list[index_2]);
		struct list_elem* last = list_begin(&list_list[index_2]);

		for(i=0;i<index_before;i++) before = list_next(before);
		for(i=0;i<index_first;i++) first = list_next(first);
		for(i=0;i<index_last;i++) last = list_next(last);

		list_splice(before,first,last);
	}

	//list_swap list_name list_index_a list_index_b
	else if(!strcmp("list_swap",tok_cmd[0]) && size == 4){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);
		int a_index = atoi(tok_cmd[2]);
		int b_index = atoi(tok_cmd[3]);

		struct list_elem* a = list_begin(&list_list[index]);
		struct list_elem* b = list_begin(&list_list[index]);

		int i;

		for(i=0;i<a_index;i++) a = list_next(a);
		for(i=0;i<b_index;i++) b = list_next(b);

		list_swap(a,b);
	}

	//list_unique list_name [list_name_duplicate]
	else if(!strcmp("list_unique",tok_cmd[0]) && (size == 3 || size == 2)){
		int index = find_index_by_name(list_name,tok_cmd[1],list_cnt);

		if(size==2) list_unique(&list_list[index],NULL,list_less,NULL);
		else{
			int index_duplicate = find_index_by_name(list_name,tok_cmd[2],list_cnt);
			list_unique(&list_list[index],&list_list[index_duplicate],list_less,NULL);
		}
	}

/*
   =============================HASH COMMANDS=============================
*/

	//hash_insert hashtable_name hash_data
	else if(!strcmp("hash_insert",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(hash_name,tok_cmd[1],hash_cnt);
		int input_data = atoi(tok_cmd[2]);

		struct hash_elem *new = (struct hash_elem*)malloc(sizeof(struct hash_elem));
		struct hash_item* new_item = hash_entry(new,struct hash_item,elem);
		new_item->data = input_data;

		new = hash_insert(&hash_list[index],new);
	}

	//hash_apply hashtable_name hash_action_func
	else if(!strcmp("hash_apply",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(hash_name,tok_cmd[1],hash_cnt);

		if(!strcmp("square",tok_cmd[2])) hash_apply(&hash_list[index],square);
		else if(!strcmp("triple",tok_cmd[2])) hash_apply(&hash_list[index],triple);
	}

	//hash_delete hashtable_name delete_data
	else if(!strcmp("hash_delete",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(hash_name,tok_cmd[1],hash_cnt);
		int delete_data = atoi(tok_cmd[2]);

		struct hash_elem *e = (struct hash_elem*)malloc(sizeof(struct hash_elem));
		struct hash_item *e_item = hash_entry(e,struct hash_item,elem);
		e_item->data = delete_data;

		hash_delete(&hash_list[index],e);
	}

	//hash_empty hashtable_name
	else if(!strcmp("hash_empty",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(hash_name,tok_cmd[1],hash_cnt);

		if(hash_empty(&hash_list[index])) printf("true\n");
		else printf("false\n");
	}

	//hash_size hashtable_name
	else if(!strcmp("hash_size",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(hash_name,tok_cmd[1],hash_cnt);

		printf("%zu\n",hash_size(&hash_list[index]));
	}

	//hash_clear hashtable_name
	else if(!strcmp("hash_clear",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(hash_name,tok_cmd[1],hash_cnt);

		hash_clear(&hash_list[index],destructor);
	}

	//hash_find hashtable_name find_data
	else if(!strcmp("hash_find",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(hash_name,tok_cmd[1],hash_cnt);
		int find_data = atoi(tok_cmd[2]);

		struct hash_elem *e = (struct hash_elem*)malloc(sizeof(struct hash_elem));
		struct hash_item *e_item = hash_entry(e,struct hash_item,elem);
		e_item->data = find_data;

		struct hash_elem *found = hash_find(&hash_list[index],e);

		if(found!=NULL) printf("%d\n",find_data);
	}

	//hash_replace hashtable_name find_data
	else if(!strcmp("hash_replace",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(hash_name,tok_cmd[1],hash_cnt);
		int find_data = atoi(tok_cmd[2]);

		struct hash_elem *e = (struct hash_elem*)malloc(sizeof(struct hash_elem));
		struct hash_item *e_item = hash_entry(e,struct hash_item,elem);
		e_item->data = find_data;

		hash_replace(&hash_list[index],e);
	}

/*
   =============================BITMAP COMMANDS=============================
*/

	//bitmap_mark bitmap_name bitmap_index
	else if(!strcmp("bitmap_mark",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t idx = (size_t)atoi(tok_cmd[2]);

		bitmap_mark(bitmap_list[index],idx);
	}

	//bitmap_all bitmap_name bitmap_index_start bitmap_index_cnt
	else if(!strcmp("bitmap_all",tok_cmd[0]) && size == 4){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t start = (size_t)atoi(tok_cmd[2]);
		size_t cnt = (size_t)atoi(tok_cmd[3]);

		if(bitmap_all(bitmap_list[index],start,cnt)) printf("true\n");
		else printf("false\n");
	}

	//bitmap_any bitmap_name bitmap_index_start bitmap_index_cnt
	else if(!strcmp("bitmap_any",tok_cmd[0]) && size == 4){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t start = (size_t)atoi(tok_cmd[2]);
		size_t cnt = (size_t)atoi(tok_cmd[3]);

		if(bitmap_any(bitmap_list[index],start,cnt)) printf("true\n");
		else printf("false\n");
	}

	//bitmap_contains bitmap_name bitmap_index_start bitmap_index_cnt bool
	else if(!strcmp("bitmap_contains",tok_cmd[0]) && size == 5){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t start = (size_t)atoi(tok_cmd[2]);
		size_t cnt = (size_t)atoi(tok_cmd[3]);
		bool bool_val = false;

		if(!strcmp("true",tok_cmd[4])) bool_val = true;

		if(bitmap_contains(bitmap_list[index],start,cnt,bool_val)) printf("true\n");
		else printf("false\n");
	}

	//bitmap_count bitmap_name bitmap_index_start bitmap_index_cnt bool
	else if(!strcmp("bitmap_count",tok_cmd[0]) && size == 5){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t start = (size_t)atoi(tok_cmd[2]);
		size_t cnt = (size_t)atoi(tok_cmd[3]);
		bool bool_val = false;

		if(!strcmp("true",tok_cmd[4])) bool_val = true;

		printf("%zu\n",(bitmap_count(bitmap_list[index],start,cnt,bool_val)));
	}

	//bitmap_dump bitmap_name
	else if(!strcmp("bitmap_dump",tok_cmd[0]) && size == 2){	
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);

		bitmap_dump(bitmap_list[index]);
	}

	//bitmap_expand bitmap_name additional_size
	else if(!strcmp("bitmap_expand",tok_cmd[0]) && size == 3){	
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		int size = atoi(tok_cmd[2]);

		bitmap_list[index] = bitmap_expand(bitmap_list[index],size);
	}

	//bitmap_set_all bitmap_name bool
	else if(!strcmp("bitmap_set_all",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		bool bool_val = false;

		if(!strcmp("true",tok_cmd[2])) bool_val = true;

		bitmap_set_all(bitmap_list[index],bool_val);
	}

	//bitmap_flip bitmap_name bitmap_index
	else if(!strcmp("bitmap_flip",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t bit_idx = (size_t)atoi(tok_cmd[2]);

		bitmap_flip(bitmap_list[index],bit_idx);
	}

	//bitmap_none bitmap_name bitmap_index_start bitmap_index_count
	else if(!strcmp("bitmap_none",tok_cmd[0]) && size == 4){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t start = (size_t)atoi(tok_cmd[2]);
		size_t cnt = (size_t)atoi(tok_cmd[3]);

		if(bitmap_none(bitmap_list[index],start,cnt)) printf("true\n");
		else printf("false\n");
	}

	//bitmap_reset bitmap_name bitmap_index
	else if(!strcmp("bitmap_reset",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t bit_idx = (size_t)atoi(tok_cmd[2]);

		bitmap_reset(bitmap_list[index],bit_idx);
	}

	//bitmap_scan_and_flip bitmap_name bitmap_index_start bitmap_index_cnt bool
	else if(!strcmp("bitmap_scan_and_flip",tok_cmd[0]) && size == 5){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t start = (size_t)atoi(tok_cmd[2]);
		size_t cnt = (size_t)atoi(tok_cmd[3]);
		bool bool_val = false;

		if(!strcmp("true",tok_cmd[4])) bool_val = true;

		printf("%zu\n",(bitmap_scan_and_flip(bitmap_list[index],start,cnt,bool_val)));
	}

	//bitmap_scan bitmap_name bitmap_index_start bitmap_index_cnt bool
	else if(!strcmp("bitmap_scan",tok_cmd[0]) && size == 5){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t start = (size_t)atoi(tok_cmd[2]);
		size_t cnt = (size_t)atoi(tok_cmd[3]);
		bool bool_val = false;

		if(!strcmp("true",tok_cmd[4])) bool_val = true;

		printf("%zu\n",(bitmap_scan(bitmap_list[index],start,cnt,bool_val)));
	}

	//bitmap_set bitmap_name bitmap_index bool
	else if(!strcmp("bitmap_set",tok_cmd[0]) && size == 4){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t bit_idx = (size_t)atoi(tok_cmd[2]);
		bool bool_val = false;

		if(!strcmp("true",tok_cmd[3])) bool_val = true;

		bitmap_set(bitmap_list[index],bit_idx,bool_val);
	}

	//bitmap_set_multiple bitmap_name bitmap_index_start bitmap_index_cnt bool
	else if(!strcmp("bitmap_set_multiple",tok_cmd[0]) && size == 5){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t start = (size_t)atoi(tok_cmd[2]);
		size_t cnt = (size_t)atoi(tok_cmd[3]);
		bool bool_val = false;

		if(!strcmp("true",tok_cmd[4])) bool_val = true;

		bitmap_set_multiple(bitmap_list[index],start,cnt,bool_val);
	}

	//bitmap_size bitmap_name
	else if(!strcmp("bitmap_size",tok_cmd[0]) && size == 2){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);

		printf("%zu\n",bitmap_size(bitmap_list[index]));
	}

	//bitmap_test bitmap_name bitmap_index
	else if(!strcmp("bitmap_test",tok_cmd[0]) && size == 3){
		int index = find_index_by_name(bitmap_name,tok_cmd[1],bitmap_cnt);
		size_t idx = (size_t)atoi(tok_cmd[2]);

		if(bitmap_test(bitmap_list[index],idx)) printf("true\n");
		else printf("false\n");
	}

	else continue;

	//end of while loop
	}

	return 0;
}

void square(struct hash_elem *e, void *aux){
	struct hash_item* e_item = hash_entry(e,struct hash_item,elem);
	e_item->data *= e_item->data;
}

void triple(struct hash_elem *e, void *aux){
	struct hash_item* e_item = hash_entry(e,struct hash_item,elem);
	e_item->data *= e_item->data * e_item->data;
}
	

//Compares the value of two list elements A and B,given AUX.
//Returns true if A is less than B, or false if A is greater than or equal to B.
bool list_less(const struct list_elem *a, const struct list_elem *b, void *aux){
	struct list_item *a_item = list_entry(a,struct list_item,elem);
	struct list_item *b_item = list_entry(b,struct list_item,elem);

	if(a_item->data < b_item->data) return true;
	else return false;
}

//find name from given name list (type given)
int find_index_by_name(char** name_list, char* name, int cnt){
	for(int i=0;i<cnt;i++) if(!strcmp(name,name_list[i])) return i;
	return -1;
}

//destructor for hash table
void destructor(struct hash_elem *e, void *aux){
	struct hash_item *e_item = hash_entry(e,struct hash_item,elem);
	free(e_item);
}

//function to print all data in the hash table, used as hash_action_func
void hash_dump_all(struct hash_elem *e, void *aux){
	struct hash_item *e_item = hash_entry(e,struct hash_item,elem);
	printf("%d ",e_item->data);
}

//function to return hash value of data in hash element, used as hash_hash_func
unsigned hash_hash(const struct hash_elem *e, void *aux){
	struct hash_item* item = hash_entry(e,struct hash_item,elem);
	return hash_int(item->data);
}

//returns true if data in a item is less than b item, false if greater than or equal to. used as hash_less_func
bool hash_less(const struct hash_elem *a, const struct hash_elem *b, void *aux){
	struct hash_item *a_item, *b_item;
	unsigned a_data, b_data;

	a_item = hash_entry(a,struct hash_item,elem);
	b_item = hash_entry(b,struct hash_item,elem);

	a_data = (a_item->data);
	b_data = (b_item->data);

	if(a_data<b_data) return true;
	else return false;
}

//function to find which type has to be found (type not given)
int find_name(char* name,char** list_name,char** hash_name,char** bitmap_name,int list_cnt, int hash_cnt, int bitmap_cnt){
	//return 1- if list, 2- if bitmap, 3- if hashtable
	int i;

	//find from list
	for(i=0;i<list_cnt;i++) if(!strcmp(name,list_name[i])) return 10+i;

	//find from hash table
	for(i=0;i<hash_cnt;i++) if(!strcmp(name,hash_name[i])) return 30+i;

	//find from bitmap
	for(i=0;i<bitmap_cnt;i++) if(!strcmp(name,bitmap_name[i])) return 20+i;

	return 0;
}
