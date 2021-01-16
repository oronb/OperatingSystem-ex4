#include "item_reporter.h"

list_node* list_head;
list_node* list_tail;

int main()
{
    int ret;
    struct item itemGiven;

    close(1);
    open_file(REPORTER_FILE);

    while(1)
    {
        ret = read(0, &itemGiven, sizeof(struct item));
        if(ret == 0)
        {
            exit(0);
        }
        printf("%s",REPORTER_MSG);
        print_one_item(&itemGiven);
    }
}

//------------------------------------------------
int open_file(char* name)
{
  int fd;
  fd = open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd < 0)  // open failed
  {
    fprintf(stderr, "ERROR: open \"%s\" failed (%d). Exiting\n", name, fd);
    exit(2);
  }
  //fprintf(stderr, "opened file %s, file descriptor is: %d\n",name, fd);
  return(fd);
}
