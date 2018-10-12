/*
  This is a silly example, but it demonstrates the use of the "memmgr" library
  in malloc/free style.
*/

#include <stdio.h>
#include <string.h>
#include <memmgr.h>

#define ENOUGH_FOR_ANYBODY 80

int main(int argc, char **argv) {
	char *buf;

	MemoryManagerOpen("shmoe");

	tape = MemMalloc(ENOUGH_FOR_ANYBODY);
	strncpy(buf, "ecoute et repete, s'il vous plait.", ENOUGH_FOR_ANYBODY - 1);
	printf("%s\n", buf);
	MemRelease(&buf);

	MemoryManagerClose("shmoe");
	return 0;
}
