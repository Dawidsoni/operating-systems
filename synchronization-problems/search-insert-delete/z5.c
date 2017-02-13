#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define THREAD_COUNT 100
#define SET_SIZE 10000
#define MAX_RAND 1000000

typedef struct list_node {
	int val;
	struct list_node* next;	
} list_node;

typedef struct clist { 
	pthread_mutex_t read_mutex;
	pthread_mutex_t append_mutex; 
	pthread_cond_t no_readers;
	pthread_cond_t no_erasers;
	unsigned reader_count;
	unsigned del_req;		
	list_node* start_node;
} clist;

void clist_init(clist* list) {
	pthread_mutex_init(&list->read_mutex, NULL);
	pthread_mutex_init(&list->append_mutex, NULL);
	pthread_cond_init(&list->no_readers, NULL);	
	pthread_cond_init(&list->no_erasers, NULL);	
	list->start_node = NULL;
	list->reader_count = 0;
	list->del_req = 0;
}

void clist_lock_search(clist* list) {
	pthread_mutex_lock(&list->read_mutex);
	if(list->del_req > 0) {//prevents delete operation starvation 
		pthread_cond_wait(&list->no_erasers, &list->read_mutex);				
	}	
	list->reader_count += 1;	
	pthread_mutex_unlock(&list->read_mutex);	
}

void clist_unlock_search(clist* list) {
	pthread_mutex_lock(&list->read_mutex);
	list->reader_count -= 1;	
	if(list->reader_count == 0) {
		pthread_cond_broadcast(&list->no_readers);				
	}
	pthread_mutex_unlock(&list->read_mutex);	
}

int clist_node_search(clist* list, int val) {
	list_node* node = list->start_node;
	while(node != NULL) {
		if(node->val == val) {
			return 1;
		}
		node = node->next;
	} 
	return 0;		
}

int clist_search(clist* list, int val) {
	clist_lock_search(list);
	int result = clist_node_search(list, val);
	clist_unlock_search(list);	
	return result;
}

void clist_lock_append(clist* list) {
	pthread_mutex_lock(&list->append_mutex);	
}

void clist_unlock_append(clist* list) {
	pthread_mutex_unlock(&list->append_mutex);				
}

list_node* clist_create_node(int val) {
	list_node* node = malloc(sizeof(list_node));
	node->val = val;
	node->next = NULL;
	return node;
}

void clist_node_append(clist* list, int val) {
	list_node* node = list->start_node;
	if(node == NULL) {
		list->start_node = clist_create_node(val);
		return;
	}	
	while(node->next != NULL) {
		node = node->next;
	}
	node->next = clist_create_node(val);	
}

void clist_append(clist* list, int val) {
	clist_lock_append(list);
	clist_node_append(list, val);
	clist_unlock_append(list);	
}

void clist_lock_delete(clist* list) {
	pthread_mutex_lock(&list->read_mutex);
	if(list->reader_count > 0) {
		list->del_req += 1;
		pthread_cond_wait(&list->no_readers, &list->read_mutex);		
		list->del_req -= 1;	
	}	
	pthread_mutex_lock(&list->append_mutex);		
}

void clist_unlock_delete(clist* list) {
	if(list->del_req == 0) {
		pthread_cond_broadcast(&list->no_erasers);				
	}	
	pthread_mutex_unlock(&list->read_mutex);	
	pthread_mutex_unlock(&list->append_mutex);				
}

void clist_node_cut(clist* list, list_node* prev_node, list_node* node) {
	if(prev_node == NULL) {
		list->start_node = node->next;
	}else {
		prev_node->next = node->next;
	}
	free(node);
}

int clist_node_delete(clist* list, int val) {
	list_node* prev_node = NULL;
	list_node* node = list->start_node;
	while(node != NULL) {		
		if(node->val == val) {
			clist_node_cut(list, prev_node, node);
			return 1;
		}
		prev_node = node;
		node = node->next;
	}
	return 0; 	
}

int clist_delete(clist* list, int val) {
	clist_lock_delete(list);
	int result = clist_node_delete(list, val);
	clist_unlock_delete(list);
	return result;
}

clist list;
int max_rand = 1000000;
pthread_t thread_arr[THREAD_COUNT];

void* reader_test() {
	while(1) {
		usleep(rand() % 500);
		int val = rand() % MAX_RAND;
		int result = clist_search(&list, val);
		printf("Searching %d, result: %d\n", val, result);
	}
	return 0;	
}

void* writer_test() {
	int set_counter = 0;
	int val_set[SET_SIZE];
	while(1) {
		usleep(rand() % 1000);
		if(rand() % 3 == 0 && set_counter < SET_SIZE) {				
			int val = rand() % MAX_RAND;
			clist_append(&list, val);
			printf("Inserting %d\n", val);			 
			val_set[set_counter] = val;
			set_counter += 1;
		}else if(set_counter > 0) {
			set_counter -= 1;			
			int result = clist_delete(&list, val_set[set_counter]);
			printf("Deleting %d, result: %d\n", val_set[set_counter], result);
		}
	}
	return 0;
}


void multi_thread_test() {
	clist_init(&list);	
	for(int i = 0; i < THREAD_COUNT; i++) {
		if(i % 2 == 0) {
			pthread_create(&thread_arr[i], NULL, reader_test, NULL);			
		}else {
			pthread_create(&thread_arr[i], NULL, writer_test, NULL);						
		}
	}	
	for(int i = 0; i < THREAD_COUNT; i++) {	
		pthread_join(thread_arr[i], NULL);		
	}			
}

void one_thread_test() {
	clist_init(&list);
	clist_append(&list, 1);
	clist_append(&list, 2);
	clist_append(&list, 3);
	printf("%d\n", clist_search(&list, 1));
	printf("%d\n", clist_search(&list, 2));
	printf("%d\n", clist_search(&list, 3));
	printf("%d\n", clist_search(&list, 4));		
	printf("%d\n", clist_delete(&list, 2));		
	printf("%d\n", clist_search(&list, 2));
	printf("%d\n", clist_search(&list, 3));			
	printf("%d\n", clist_delete(&list, 1));
	printf("%d\n", clist_search(&list, 3));						
	printf("%d\n", clist_delete(&list, 3));
	printf("%d\n", clist_search(&list, 3));							
	printf("%d\n", clist_delete(&list, 3));
}



int main() {
	srand(time(NULL));
	multi_thread_test();
	return 0;
}
