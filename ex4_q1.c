#include "ex4_q1.h"

/*
pthread_mutex_t mtx_count = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_list = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_rand = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_print = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t count_thread_created = PTHREAD_COND_INITIALIZER;
pthread_cond_t count_nodes_added_to_list = PTHREAD_COND_INITIALIZER;
pthread_cond_t check_if_items_to_handle_exists = PTHREAD_COND_INITIALIZER;

int num_of_threads_created=0;
int num_of_items_create=0;
int num_of_messages_in_list=0;
int num_of_proccessed_in_list=0;
*/

//---------------------------------
// *** SEMAPHORES
#define SEM_BINARY 1
#define SEM_WAIT_ALL_THREAD_CREATED 0

//---------------------------------
// binary semaphores as global variables
sem_t* sem_list;
sem_t* sem_rand;
sem_t* sem_print;
sem_t* sem_count;
// conditional semaphores as global variables
sem_t* sem_wait_all_thread_created;
sem_t* sem_wait_for_all_items;
sem_t* sem_wait_if_no_item_to_handle;
sem_t* sem_num_of_messages_in_list; //Num of items in list
sem_t* sem_num_of_items_create;
sem_t* sem_num_of_proccessed_in_list;

void wait_for_threads_to_finish(pthread_t* threads, int num_of_threads)
{
    for(int i=0; i < num_of_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

void allocate_sem()
{
    sem_list = (sem_t*)malloc(sizeof(sem_t));
    sem_rand = (sem_t*)malloc(sizeof(sem_t));
    sem_print = (sem_t*)malloc(sizeof(sem_t));
    sem_count = (sem_t*)malloc(sizeof(sem_t));

    sem_wait_all_thread_created = (sem_t*)malloc(sizeof(sem_t));
    sem_wait_for_all_items = (sem_t*)malloc(sizeof(sem_t));
    sem_wait_if_no_item_to_handle = (sem_t*)malloc(sizeof(sem_t));
    sem_num_of_messages_in_list = (sem_t*)malloc(sizeof(sem_t));
    sem_num_of_items_create = (sem_t*)malloc(sizeof(sem_t));
    sem_num_of_proccessed_in_list = (sem_t*)malloc(sizeof(sem_t));            
}

void free_sem()
{
    free(sem_list);
    free(sem_rand);
    free(sem_print);
    free(sem_count);

    free(sem_wait_all_thread_created);
    free(sem_wait_for_all_items);
    free(sem_wait_if_no_item_to_handle);
    free(sem_num_of_messages_in_list);
    free(sem_num_of_items_create);
    free(sem_num_of_proccessed_in_list);
}

int main()
{
    pthread_t producers[N_PROD];
    pthread_t consumers[N_CONS];
    int test;

    allocate_sem();
    open_all_sem();

    /*sem_getvalue(sem_list,&test);
    printf("test_before_post:%d\n",test);
    sem_post(sem_list);
    sem_post(sem_list);
    sem_getvalue(sem_list,&test);
    printf("test_after_post:%d\n",test);*/

   // sleep(10);

    create_producers(producers);
    create_consumers(consumers);
    sleep(4);
    sem_post(sem_wait_all_thread_created);

    //Waiting for threads to finish
    wait_for_threads_to_finish(producers, N_PROD);
    wait_for_threads_to_finish(consumers, N_CONS);

    print_list();
    close_semaphores();
    unlink_semaphores();
    //destroy_mutex();
    //destroy_cond();

    printf(PROD_TERMINATED);
    printf(CONS_TERMINATED);

    free_sem();
    free_list();

}

/*void wait_until_all_thread_created()
{
    while(num_of_threads_created < (N_CONS + N_PROD))
    {
        pthread_cond_wait(&count_thread_created, &mtx_count);
    }
}*/

void * producer(void *ptr)
{
    int * thread_num = (int*) ptr;
    int num_of_items_create = 0;
    //wait_until_all_thread_created
    sem_wait(sem_wait_all_thread_created);
    sem_post(sem_wait_all_thread_created);
    int randNums[2];
    struct item* new_item = NULL;
    printf("A1");
    sem_wait(sem_count);
    sem_getvalue(sem_num_of_items_create, &num_of_items_create);
    printf("num_of_items_create:%d\n",num_of_items_create);
    while(num_of_items_create < TOTAL_ITEMS)
    {
        sem_post(sem_count);
        getting_random_numbers(randNums);
        create_item_with_lock(randNums, &new_item, thread_num);
        adding_item_to_list_with_lock(thread_num, new_item);
        sem_wait(sem_count);
        sem_getvalue(sem_num_of_items_create, &num_of_items_create);
        printf("num_of_items_create:%d\n",num_of_items_create);
    }
    sem_post(sem_count);
    end_producer(thread_num); 

    pthread_exit(NULL);

}

void * consumer(void *ptr)
{
    int * thread_num = (int*) ptr;
    wait_for_enough_items_in_list();
    handle_getting_item(thread_num);
    end_consumer(thread_num);
    pthread_exit(NULL);
}


void handle_getting_item(int * thread_num)
{
    int num_of_proccessed_in_list = 0;
    int num_of_messages_in_list = 0;
    sem_wait(sem_count);
    sem_getvalue(sem_num_of_proccessed_in_list, &num_of_proccessed_in_list);
    while(num_of_proccessed_in_list < TOTAL_ITEMS)
    {
        sem_post(sem_count);
        wait_if_no_items_to_handle();
        sem_wait(sem_count);
        sem_getvalue(sem_num_of_proccessed_in_list, &num_of_proccessed_in_list);
        sem_getvalue(sem_num_of_messages_in_list, &num_of_messages_in_list);
        if(num_of_proccessed_in_list<TOTAL_ITEMS && num_of_proccessed_in_list != num_of_messages_in_list)
        {
            sem_wait(sem_list);
            get_and_handle_item_in_list(thread_num);
            sem_post(sem_list);
        }
        sem_post(sem_count);
        sem_wait(sem_count);
        sem_getvalue(sem_num_of_proccessed_in_list, &num_of_proccessed_in_list);
	printf("$$num_of_proccessed_in_list:%d\n", num_of_proccessed_in_list);
    }
    sem_post(sem_count);

}

void update_new_item_fields(int* randNums, struct item* new_item)
{
    new_item->prod = randNums[0] * randNums[1];
    new_item->status = NOT_DONE;
}

void create_item_with_lock(int* randNums, struct item** new_item, int* thread_num)
{
    int num_of_items_create = 0;
    int test = 0;
    sem_wait(sem_count);
    sem_getvalue(sem_num_of_items_create, &num_of_items_create);
    if(num_of_items_create < TOTAL_ITEMS)
    {
        *new_item = (struct item*) malloc(sizeof(struct item));
        update_new_item_fields(randNums, *new_item);
        printf("Before posting sem_num_of_items_create:%d\n",num_of_items_create);
        sem_post(sem_num_of_items_create);
        sem_getvalue(sem_num_of_items_create, &test);
        printf("sem_num_of_items_create value:%d\n", test);
        sem_post(sem_count);
    }
    else
    {
        sem_post(sem_count);
        end_producer(thread_num);
    }
}



void check_and_wake_consumers()
{
    int num_of_items_create_val = 0;
    int num_of_messages_in_list = 0;
    int num_of_proccessed_in_list = 0;

    sem_wait(sem_count);
    sem_getvalue(sem_num_of_items_create, &num_of_items_create_val);
    printf("in check_and_wake_consumers and num_of_items_create_val is:%d\n", num_of_items_create_val);
    if(num_of_items_create_val >= ITEM_START_CNT)
    {
        sem_post(sem_wait_for_all_items);
    }
    sem_post(sem_count);

    sem_wait(sem_count);
    sem_getvalue(sem_num_of_items_create, &num_of_messages_in_list);
    sem_getvalue(sem_num_of_proccessed_in_list, &num_of_proccessed_in_list);
    if(num_of_messages_in_list != num_of_proccessed_in_list)
    {
        sem_post(sem_wait_if_no_item_to_handle);
    }
    sem_post(sem_count);
}

void wait_for_enough_items_in_list()
{
    printf("B1\n");
    sem_wait(sem_wait_for_all_items);
    printf("B2\n");
    sem_post(sem_wait_for_all_items);
    /*while(num_of_messages_in_list < ITEM_START_CNT)
    {
        pthread_cond_wait(&count_nodes_added_to_list, &mtx_count);
    }*/

}

void wait_if_no_items_to_handle()
{
    int num_of_messages_in_list = 0;
    int num_of_proccessed_in_list = 0;
    int num_of_items_create = 0;

    sem_wait(sem_count);
    sem_getvalue(sem_num_of_messages_in_list, &num_of_messages_in_list);
    sem_getvalue(sem_num_of_proccessed_in_list, &num_of_proccessed_in_list);
    sem_getvalue(sem_num_of_items_create, &num_of_items_create);
    while(num_of_proccessed_in_list == num_of_messages_in_list && num_of_proccessed_in_list < TOTAL_ITEMS)
    {
        sem_post(sem_count);
        sem_wait(sem_wait_if_no_item_to_handle);
        sem_post(sem_wait_if_no_item_to_handle);
        sem_wait(sem_count);
        sem_getvalue(sem_num_of_messages_in_list, &num_of_messages_in_list);
        sem_getvalue(sem_num_of_proccessed_in_list, &num_of_proccessed_in_list);
        sem_getvalue(sem_num_of_items_create, &num_of_items_create);
    }
    sem_post(sem_count);
}

void get_and_handle_item_in_list(int* thread_num)
{
    item * item_got = get_undone_from_list();
    write_getting_item_with_lock(thread_num,item_got);
    set_two_factors(item_got);
    item_got->status = DONE;
    sem_post(sem_num_of_proccessed_in_list);
   // num_of_proccessed_in_list++;
}



void adding_item_to_list_with_lock(int* thread_num, struct item* new_item)
{
   // pthread_mutex_lock(&mtx_count);
   int num_of_messages_in_list = 0;
   sem_wait(sem_count);
   sem_getvalue(sem_num_of_messages_in_list, &num_of_messages_in_list);
    if(num_of_messages_in_list==TOTAL_ITEMS)
    {
        sem_post(sem_count);
        end_producer(thread_num);
    }
    else
    {
        sem_post(sem_count);
        sem_wait(sem_list);
        write_adding_item_with_lock(thread_num, new_item);
        add_to_list(new_item);
        sem_post(sem_num_of_messages_in_list);
        sem_post(sem_list);
        check_and_wake_consumers();
    }    
}

void getting_random_numbers(int* randNums)
{
    for(int i=0; i<2; i++)
    {
        sem_wait(sem_rand);
        randNums[i] = get_random_in_range();
        sem_post(sem_rand);
        while(!is_prime(randNums[i]))
        {
            sem_wait(sem_rand);
            randNums[i] = get_random_in_range();
            sem_post(sem_rand);
        }
    }
}

//end threads functions
void end_producer(int * thread_num)
{
    write_producer_is_done_with_lock(thread_num);
    free(thread_num);
    pthread_exit(NULL);
}

void end_consumer(int * thread_num)
{
    write_consumer_is_done_with_lock(thread_num);
    free(thread_num);
    pthread_exit(NULL);
}

//print functions
void write_producer_is_done_with_lock(int * thread_num)
{
    sem_wait(sem_print);
    write_producer_is_done(*thread_num);
    sem_post(sem_print);
}

void write_consumer_is_done_with_lock(int * thread_num)
{
    sem_wait(sem_print);
    write_consumer_is_done(*thread_num);
    sem_post(sem_print);
}

void write_getting_item_with_lock(int * thread_num, struct item* item_got)
{
    sem_wait(sem_print);
    write_getting_item(*thread_num, item_got);
    sem_post(sem_print);
}

void write_adding_item_with_lock(int * thread_num, struct item* new_item)
{
    sem_wait(sem_print);
    write_adding_item(*thread_num, new_item);
    sem_post(sem_print);
}

//destory functions

void free_list()
{
    struct list_node* currNode = list_head;
    struct list_node* tempNode = NULL;
    while(currNode != NULL)
    {
        tempNode = currNode->next;
        free(currNode->item);
        free(currNode);
        currNode = tempNode;
    }
}

//creating threads functions
void create_producers(pthread_t* producers)
{
    //Creating producer threads
    for(int i=0; i < N_PROD; i++)
    {
       // sleep(1);
        printf("Creating producer:%d\n", i);
        int * thread_num = malloc(sizeof(int));
        *thread_num = i+1;
        pthread_create( &(producers[i]), NULL, producer, thread_num);
        //num_of_threads_created++;
    }
    printf(ALL_PROD_CREATED);
}

void create_consumers(pthread_t* consumers)
{
    //Creating consumer threads
    for(int i=0; i < N_CONS; i++)
    {
       // sleep(1);
        printf("Create consumer:%d\n", i);
        int * thread_num = malloc(sizeof(int));
        *thread_num = i+1;
        pthread_create( &(consumers[i]), NULL, consumer, thread_num);
        //num_of_threads_created++;
    }
    printf(ALL_CONS_CREATED);
}

void close_semaphores()
{
    sem_close(sem_wait_all_thread_created);
    sem_close(sem_list);
    sem_close(sem_rand);
    sem_close(sem_print);
    sem_close(sem_wait_for_all_items);
    sem_close(sem_wait_if_no_item_to_handle);
    sem_close(sem_num_of_messages_in_list);
    sem_close(sem_num_of_items_create);
    sem_close(sem_num_of_proccessed_in_list);
}

//=================================================================
void open_all_sem()
{
    sem_init(sem_list,0, SEM_BINARY);
    sem_init(sem_rand, 0, SEM_BINARY);
    sem_init(sem_print, 0, SEM_BINARY);
    sem_init(sem_count, 0, SEM_BINARY);

    sem_init(sem_wait_all_thread_created,0, 0);
    sem_init(sem_wait_for_all_items, 0, 0);
    sem_init(sem_wait_if_no_item_to_handle, 0, 0);
    sem_init(sem_num_of_messages_in_list, 0, 0);
    sem_init(sem_num_of_items_create, 0, 0);
    sem_init(sem_num_of_proccessed_in_list, 0, 0);

    /*if (sem_unlink("/sem_wait_all_thread_created")==0)
	fprintf(stderr, "successul unlink of /sem_wait_all_thread_created\n");
	sem_wait_all_thread_created = sem_open("/sem_wait_all_thread_created", O_CREAT, S_IRWXU, SEM_WAIT_ALL_THREAD_CREATED); 
	if (sem_wait_all_thread_created == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_wait_all_thread_created\n");
		exit(EXIT_FAILURE);
	}

	if (sem_unlink("/sem_list")==0)
		fprintf(stderr, "successul unlink of /sem_list\n");
	sem_list = sem_open("/sem_list", O_CREAT, S_IRWXU, SEM_BINARY); 
	if (sem_list == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_list\n");
		exit(EXIT_FAILURE);
	}

	if (sem_unlink("/sem_rand")==0)
		fprintf(stderr, "successul unlink of /sem_rand\n");
	sem_rand = sem_open("/sem_rand", O_CREAT, S_IRWXU, SEM_BINARY); 
	if (sem_rand == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_rand\n");
		exit(EXIT_FAILURE);
	}	

	if (sem_unlink("/sem_print")==0)
		fprintf(stderr, "successul unlink of /sem_print\n");
	sem_print = sem_open("/sem_print", O_CREAT, S_IRWXU, SEM_BINARY); 
	if (sem_print == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_print\n");
		exit(EXIT_FAILURE);
	}	

    if (sem_unlink("/sem_wait_for_all_items")==0)
	fprintf(stderr, "successul unlink of /sem_wait_for_all_items\n");
	sem_wait_for_all_items = sem_open("/sem_wait_for_all_items", O_CREAT, S_IRWXU, 0); 
	if (sem_wait_for_all_items == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_wait_for_all_items\n");
		exit(EXIT_FAILURE);
	}	

    if (sem_unlink("/sem_wait_if_no_item_to_handle")==0)
	fprintf(stderr, "successul unlink of /sem_wait_if_no_item_to_handle\n");
	sem_wait_if_no_item_to_handle = sem_open("/sem_wait_if_no_item_to_handle", O_CREAT, S_IRWXU, 0); 
	if (sem_wait_if_no_item_to_handle == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_wait_if_no_item_to_handle\n");
		exit(EXIT_FAILURE);
	}	

    if (sem_unlink("/sem_count")==0)
	fprintf(stderr, "successul unlink of /sem_count\n");
	sem_count = sem_open("/sem_count", O_CREAT, S_IRWXU, SEM_BINARY); 
	if (sem_count == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_count\n");
		exit(EXIT_FAILURE);
	}


    if (sem_unlink("/sem_num_of_items_create")==0)
	fprintf(stderr, "successul unlink of /sem_num_of_items_create\n");
	sem_num_of_items_create = sem_open("/sem_num_of_items_create", O_CREAT, S_IRWXU, 0); 
	if (sem_num_of_items_create == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_num_of_items_create\n");
		exit(EXIT_FAILURE);
	}	

    if (sem_unlink("/sem_num_of_messages_in_list")==0)
	fprintf(stderr, "successul unlink of /sem_num_of_messages_in_list\n");
	sem_num_of_messages_in_list = sem_open("/sem_num_of_messages_in_list", O_CREAT, S_IRWXU, 0); 
	if (sem_num_of_messages_in_list == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_num_of_messages_in_list\n");
		exit(EXIT_FAILURE);
	}		

    if (sem_unlink("/sem_num_of_proccessed_in_list")==0)
	fprintf(stderr, "successul unlink of /sem_num_of_proccessed_in_list\n");
	sem_num_of_proccessed_in_list = sem_open("/sem_num_of_proccessed_in_list", O_CREAT, S_IRWXU, 0); 
	if (sem_num_of_proccessed_in_list == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_num_of_proccessed_in_list\n");
		exit(EXIT_FAILURE);
	}	
    */
}

void unlink_semaphores()
{
    char* sem[] ={"/sem_wait_all_thread_created", "/sem_list", "/sem_rand", "/sem_print", "sem_wait_for_all_items", "sem_wait_if_no_item_to_handle","sem_num_of_messages_in_list","sem_num_of_items_create","sem_num_of_proccessed_in_list"};
	int i;
	printf("going to unlink semaphores..\n");	
	for (i=0; i<3; i++)
		if (sem_unlink(sem[i])==0)
			fprintf(stderr, "successul unlink of %s\n", sem[i]);
}
