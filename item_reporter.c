#include "item_reporter.h"

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
        printf(REPORTER_MSG , "  ");
        print_one_item(&itemGiven);
    }
}