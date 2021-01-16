#include "item_reporter.h"

int main()
{
  //int pip[2];
    int ret;
    struct item itemGiven;
    //pipe(pip);
    //close(pip[1]);
    while(1)
    {
        int fd = dup(0);
        ret = read(fd, &itemGiven, sizeof(struct item));
        if(ret == 0)
        {
            exit(0);
        }
        printf("@@@prod%d\n\n",itemGiven.prod);
        //sleep(10);
    }
}