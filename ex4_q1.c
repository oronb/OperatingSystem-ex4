#include "ex4_q1.h"

//---------------------------------


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

int pip[2];

void wait_for_threads_to_finish(pthread_t* threads, int num_of_threads)
{
    for(int i=0; i < num_of_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

int main()
{
    pthread_t producers[N_PROD];
    pthread_t consumers[N_CONS];

    unlink_semaphores();

    open_all_sem();

    create_producers(producers);
    create_consumers(consumers);
    pipe(pip);
    handle_item_reporter();
    close(pip[0]);
    sem_post(sem_wait_all_thread_created);

    //Waiting for threads to finish
    wait_for_threads_to_finish(producers, N_PROD);
    wait_for_threads_to_finish(consumers, N_CONS);

    print_list();
    close_semaphores();

    printf(PROD_TERMINATED);
    printf(CONS_TERMINATED);

    free_list();

}

void * producer(void *ptr)
{
    int * thread_num = (int*) ptr;
    int num_of_items_create = 0;
    sem_wait(sem_wait_all_thread_created);
    sem_post(sem_wait_all_thread_created);
    int randNums[2];
    struct item* new_item = NULL;
    sem_wait(sem_count);
    sem_getvalue(sem_num_of_items_create, &num_of_items_create);
    while(num_of_items_create < TOTAL_ITEMS)
    {
        sem_post(sem_count);
        getting_random_numbers(randNums);
        create_item_with_lock(randNums, &new_item, thread_num);
        adding_item_to_list_with_lock(thread_num, new_item);
        sem_wait(sem_count);
        sem_getvalue(sem_num_of_items_create, &num_of_items_create);
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

void handle_item_reporter()
{
    char* my_argv[2];
    my_argv[0] = ITEM_REPORTER_PROG;
    my_argv[1] = NULL;

    if (fork() == 0)
    {
      close(0);
      dup(pip[0]);
      close(pip[1]);
      execve(ITEM_REPORTER_PROG, my_argv, NULL);
      fprintf(stderr, "*** ERROR: *** EXEC of %s FAILED\n", ITEM_REPORTER_PROG);
      exit(1);
    } 
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
    sem_wait(sem_count);
    sem_getvalue(sem_num_of_items_create, &num_of_items_create);
    if(num_of_items_create < TOTAL_ITEMS)
    {
        *new_item = (struct item*) malloc(sizeof(struct item));
        update_new_item_fields(randNums, *new_item);
        sem_post(sem_num_of_items_create);
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
    sem_wait(sem_wait_for_all_items);
    sem_post(sem_wait_for_all_items);
}

void wait_if_no_items_to_handle()
{
    int num_of_messages_in_list = 0;
    int num_of_proccessed_in_list = 0;

    sem_wait(sem_count);
    sem_getvalue(sem_num_of_messages_in_list, &num_of_messages_in_list);
    sem_getvalue(sem_num_of_proccessed_in_list, &num_of_proccessed_in_list);
    while(num_of_proccessed_in_list == num_of_messages_in_list && num_of_proccessed_in_list < TOTAL_ITEMS)
    {
        sem_post(sem_count);
        sem_wait(sem_wait_if_no_item_to_handle);
        sem_post(sem_wait_if_no_item_to_handle);
        sem_wait(sem_count);
        sem_getvalue(sem_num_of_messages_in_list, &num_of_messages_in_list);
        sem_getvalue(sem_num_of_proccessed_in_list, &num_of_proccessed_in_list);
    }
    sem_post(sem_count);
}

void get_and_handle_item_in_list(int* thread_num)
{
    item * item_got = get_undone_from_list();
    write_getting_item_with_lock(thread_num,item_got);
    set_two_factors(item_got);
    item_got->status = DONE;
    write(pip[1],item_got,sizeof(*item_got));
    sem_post(sem_num_of_proccessed_in_list);
}

void adding_item_to_list_with_lock(int* thread_num, struct item* new_item)
{
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
        int * thread_num = malloc(sizeof(int));
        *thread_num = i+1;
        pthread_create( &(producers[i]), NULL, producer, thread_num);
    }
    printf(ALL_PROD_CREATED);
}

void create_consumers(pthread_t* consumers)
{
    //Creating consumer threads
    for(int i=0; i < N_CONS; i++)
    {
        int * thread_num = malloc(sizeof(int));
        *thread_num = i+1;
        pthread_create( &(consumers[i]), NULL, consumer, thread_num);
    }
    printf(ALL_CONS_CREATED);
}

void close_semaphores()
{
    sem_close(sem_list);
    sem_close(sem_rand);
    sem_close(sem_print);
    sem_close(sem_count);

    sem_close(sem_wait_all_thread_created);
    sem_close(sem_wait_for_all_items);
    sem_close(sem_wait_if_no_item_to_handle);
    sem_close(sem_num_of_messages_in_list);
    sem_close(sem_num_of_items_create);
    sem_close(sem_num_of_proccessed_in_list);
}


// conditional semaphores as global variables
sem_t* sem_wait_all_thread_created;
sem_t* sem_wait_for_all_items;
sem_t* sem_wait_if_no_item_to_handle;
sem_t* sem_num_of_messages_in_list; //Num of items in list
sem_t* sem_num_of_items_create;
sem_t* sem_num_of_proccessed_in_list;


//=================================================================
void open_all_sem()
{
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

        if (sem_unlink("/sem_count")==0)
	    fprintf(stderr, "successul unlink of /sem_count\n");
	sem_count = sem_open("/sem_count", O_CREAT, S_IRWXU, SEM_BINARY); 
	if (sem_count == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_count\n");
		exit(EXIT_FAILURE);
	}

    if (sem_unlink("/sem_wait_all_thread_created")==0)
	    fprintf(stderr, "successul unlink of /sem_wait_all_thread_created\n");
	sem_wait_all_thread_created = sem_open("/sem_wait_all_thread_created", O_CREAT, S_IRWXU, 0); 
	if (sem_wait_all_thread_created == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_wait_all_thread_created\n");
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

    if (sem_unlink("/sem_num_of_messages_in_list")==0)
	    fprintf(stderr, "successul unlink of /sem_num_of_messages_in_list\n");
	sem_num_of_messages_in_list = sem_open("/sem_num_of_messages_in_list", O_CREAT, S_IRWXU, 0); 
	if (sem_num_of_messages_in_list == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_num_of_messages_in_list\n");
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

    if (sem_unlink("/sem_num_of_proccessed_in_list")==0)
	    fprintf(stderr, "successul unlink of /sem_num_of_proccessed_in_list\n");
	sem_num_of_proccessed_in_list = sem_open("/sem_num_of_proccessed_in_list", O_CREAT, S_IRWXU, 0); 
	if (sem_num_of_proccessed_in_list == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_num_of_proccessed_in_list\n");
		exit(EXIT_FAILURE);
	}	
    
}

void unlink_semaphores()
{
    char* sem[] ={"/sem_list", "/sem_rand", "/sem_print", "sem_count", "/sem_wait_all_thread_created", "sem_wait_for_all_items", "sem_wait_if_no_item_to_handle","sem_num_of_messages_in_list","sem_num_of_items_create","sem_num_of_proccessed_in_list"};
	int i;
	printf("going to unlink semaphores..\n");	
	for (i=0; i<10; i++)
		if (sem_unlink(sem[i])==0)
			fprintf(stderr, "successul unlink of %s\n", sem[i]);
}