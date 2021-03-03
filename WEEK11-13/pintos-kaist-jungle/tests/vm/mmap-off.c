/* Tries to mmap with offset > 0. */

#include <syscall.h>
#include <string.h>
#include "tests/lib.h"
#include "tests/main.h"
#include "tests/vm/large.inc"
static char zeros[0x1000];

void
test_main (void) 
{
  int handle;
  char buf[0x1000];
// printf("#########################test start !!##########################\n ");
  CHECK ((handle = open ("large.txt")) > 1, "open \"large.txt\"");
//   printf("------------------------->this is address 1 %p\n",process_get_file(handle));
    // printf("what is large's length? -> %d \n", strlen(large));
    // printf("--> 1. what is fd ? %d \n", handle);
  CHECK (mmap ((void *) 0x10000000, 4096, 1, handle, 0x1000) == (void *) 0x10000000,
          "try to mmap with offset 0x1000");
//   printf("--------------->this is zero : %s.\n", zeros);

  close (handle);
    // printf("--------------->*this is zero : %s, /// %s\n", zeros, 0x10000000);


  msg ("validate mmap.");
  if (!memcmp ((void *) 0x10000000, &large[0x1000], 0x1000))
      msg ("validated.");
  else
      fail ("validate fail.");

  msg ("write to mmap");
//   printf("--------------->this is zero : %s\n", zeros);
  memset (zeros, 0, 0x1000);
  memcpy ((void *) 0x10000000, zeros, 0x1000);
//   printf("--------------->this is zero : %s\n", zeros);

  munmap ((void *) 0x10000000);

  msg ("validate contents.");

  CHECK ((handle = open ("large.txt")) > 1, "open \"large.txt\"");
//   printf("------------------------->this is address 1 %p\n",process_get_file(handle));

//   printf("--> 2. what is fd ? %d \n", handle);
//   printf("--------->what is read func return? -->%d\n", read (handle, buf, 0x1000));
  CHECK (0x1000 == read (handle, buf, 0x1000), "read \"large.txt\" Page 0");

  msg ("validate page 0.");
  if (!memcmp ((void *) buf, large, 0x1000))
      msg ("validated.");
  else
      fail ("validate fail.");

  CHECK (0x1000 == read (handle, buf, 0x1000), "read \"large.txt\" Page 1");

  msg ("validate page 1.");
  if (!memcmp ((void *) buf, zeros, 0x1000))
      msg ("validated.");
  else
      fail ("validate fail.");
  close (handle);

  msg ("success");
}
