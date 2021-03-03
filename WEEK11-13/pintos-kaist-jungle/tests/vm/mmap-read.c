/* Uses a memory mapping to read a file. */

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  char *actual = (char *) 0x10000000;
  int handle;
  void *map;
  size_t i;
    // printf("------------->here 1<---------------\n");
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
    // printf("------------->here 2<---------------\n");

  CHECK ((map = mmap (actual, 4096, 0, handle, 0)) != MAP_FAILED, "mmap \"sample.txt\"");
    // printf("map address is %p\n",map);
    // printf("------------->here !<---------------\n");

  /* Check that data is correct. */
  if (memcmp (actual, sample, strlen (sample))){
    fail ("read of mmap'd file reported bad data");
    // printf("memcmp is falt!!!\n");
  }
// printf("------------->here 3<---------------\n");

  /* Verify that data is followed by zeros. */
  for (i = strlen (sample); i < 4096; i++)
    if (actual[i] != 0)
      fail ("byte %zu of mmap'd region has value %02hhx (should be 0)",
            i, actual[i]);
    // printf("------------->here 4<---------------\n");

  munmap (map);
    // printf("------------->here 5<---------------\n");

  close (handle);
}
