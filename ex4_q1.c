#include "ex4_q1.h"

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

    create_producers(producers);
    create_consumers(consumers);
    pthread_cond_broadcast(&count_thread_created); //TODO: check if it releases all threads

    //Waiting for threads to finish

    wait_for_threads_to_finish(producers, N_PROD);
    wait_for_threads_to_finish(consumers, N_CONS);

    print_list();
    destroy_mutex();
    destroy_cond();

    printf(PROD_TERMINATED);
    printf(CONS_TERMINATED);

    free_list();

}

void wait_until_all_thread_created()
{
    pthread_mutex_lock(&mtx_count);
    while(num_of_threads_created < (N_CONS + N_PROD))
    {
        pthread_cond_wait(&count_thread_created, &mtx_count);
    }
    pthread_mutex_unlock(&mtx_count);
}

void * producer(void *ptr)
{
    int * thread_num = (int*) ptr;
    wait_until_all_thread_created();

    int randNums[2];
    struct item* new_item = NULL;
    pthread_mutex_lock(&mtx_count);
    while(num_of_items_create < TOTAL_ITEMS)
    {
        pthread_mutex_unlock(&mtx_count);                
        getting_random_numbers(randNums);
        create_item_with_lock(randNums, &new_item, thread_num);
        adding_item_to_list_with_lock(thread_num, new_item);
        pthread_mutex_lock(&mtx_count);
    }
    pthread_mutex_unlock(&mtx_count);

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
    pthread_mutex_lock(&mtx_count);
    while(num_of_proccessed_in_list < TOTAL_ITEMS)
    {
        pthread_mutex_unlock(&mtx_count);
        wait_if_no_items_to_handle();
        pthread_mutex_lock(&mtx_count);
        if(num_of_proccessed_in_list<TOTAL_ITEMS && num_of_proccessed_in_list != num_of_messages_in_list)
        {
            pthread_mutex_lock(&mtx_list);
            get_and_handle_item_in_list(thread_num);
            pthread_mutex_unlock(&mtx_list);
        }
        pthread_mutex_unlock(&mtx_count);
        pthread_mutex_lock(&mtx_count);
    }
    pthread_mutex_unlock(&mtx_count);

}

void update_new_item_fields(int* randNums, struct item* new_item)
{
    new_item->prod = randNums[0] * randNums[1];
    new_item->status = NOT_DONE;
}

void create_item_with_lock(int* randNums, struct item** new_item, int* thread_num)
{
    pthread_mutex_lock(&mtx_count);
    if(num_of_items_create < TOTAL_ITEMS)
    {
        *new_item = (struct item*) malloc(sizeof(struct item));
        update_new_item_fields(randNums, *new_item);
        num_of_items_create++;
        pthread_mutex_unlock(&mtx_count);
    }
    else
    {
        pthread_mutex_unlock(&mtx_count);
        end_producer(thread_num);
    }
}

void check_and_wake_consumers()
{
    pthread_mutex_lock(&mtx_count);
    if(num_of_items_create >= ITEM_START_CNT)
    {
        pthread_cond_broadcast(&count_nodes_added_to_list);
    }
    pthread_mutex_unlock(&mtx_count);

    pthread_mutex_lock(&mtx_count);
    if(num_of_messages_in_list != num_of_proccessed_in_list)
    {
        pthread_cond_broadcast(&check_if_items_to_handle_exists);
    }
    pthread_mutex_unlock(&mtx_count);
}

void wait_for_enough_items_in_list()
{
    pthread_mutex_lock(&mtx_count);
    while(num_of_messages_in_list < ITEM_START_CNT)
    {
        pthread_cond_wait(&count_nodes_added_to_list, &mtx_count);
    }
    pthread_mutex_unlock(&mtx_count);

}

void wait_if_no_items_to_handle()
{
    pthread_mutex_lock(&mtx_count);
    while(num_of_proccessed_in_list == num_of_messages_in_list && num_of_proccessed_in_list < TOTAL_ITEMS)
    {
        pthread_cond_wait(&check_if_items_to_handle_exists, &mtx_count);
    }
    pthread_mutex_unlock(&mtx_count);
}

void get_and_handle_item_in_list(int* thread_num)
{
    item * item_got = get_undone_from_list();
    write_getting_item_with_lock(thread_num,item_got);
    set_two_factors(item_got);
    item_got->status = DONE;
    num_of_proccessed_in_list++;
}


void adding_item_to_list_with_lock(int* thread_num, struct item* new_item)
{
    pthread_mutex_lock(&mtx_count);
    if(num_of_messages_in_list==TOTAL_ITEMS)
    {
        pthread_mutex_unlock(&mtx_count);
        end_producer(thread_num);
    }
    else
    {
        pthread_mutex_lock(&mtx_list);
        write_adding_item_with_lock(thread_num, new_item);
        add_to_list(new_item);
        num_of_messages_in_list++;
        pthread_mutex_unlock(&mtx_count);
        pthread_mutex_unlock(&mtx_list);
        check_and_wake_consumers();
    }    
}

void getting_random_numbers(int* randNums)
{
    for(int i=0; i<2; i++)
    {
        pthread_mutex_lock(&mtx_rand);
        randNums[i] = get_random_in_range();
        pthread_mutex_unlock(&mtx_rand);
        while(!is_prime(randNums[i]))
        {
            pthread_mutex_lock(&mtx_rand);
            randNums[i] = get_random_in_range();
            pthread_mutex_unlock(&mtx_rand);
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
    pthread_mutex_lock(&mtx_print);
    write_producer_is_done(*thread_num);
    pthread_mutex_unlock(&mtx_print);
}

void write_consumer_is_done_with_lock(int * thread_num)
{
    pthread_mutex_lock(&mtx_print);
    write_consumer_is_done(*thread_num);
    pthread_mutex_unlock(&mtx_print);
}

void write_getting_item_with_lock(int * thread_num, struct item* item_got)
{
    pthread_mutex_lock(&mtx_print);
    write_getting_item(*thread_num, item_got);
    pthread_mutex_unlock(&mtx_print);
}

void write_adding_item_with_lock(int * thread_num, struct item* new_item)
{
    pthread_mutex_lock(&mtx_print);
    write_adding_item(*thread_num, new_item);
    pthread_mutex_unlock(&mtx_print);
}

//destory functions
void destroy_mutex()
{
    pthread_mutex_destroy(&mtx_list);
    pthread_mutex_destroy(&mtx_rand);
    pthread_mutex_destroy(&mtx_print);
    pthread_mutex_destroy(&mtx_count);
}

void destroy_cond()
{
    pthread_cond_destroy(&count_thread_created);
    pthread_cond_destroy(&count_nodes_added_to_list);
    pthread_cond_destroy(&check_if_items_to_handle_exists);
}

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
        num_of_threads_created++;
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
        num_of_threads_created++;
    }
    printf(ALL_CONS_CREATED);
}