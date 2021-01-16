#include "item_reporter.h"

list_node* list_head;
list_node* list_tail;

int main()
{
    int ret;
    struct item itemGiven;
    while(1)
    {
        ret = read(0, &itemGiven, sizeof(struct item));
        if(ret == 0)
        {
            exit(0);
        }
        printf("%s ",REPORTER_MSG);
        print_one_item(&itemGiven);
    }
}