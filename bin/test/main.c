#include <conio.h>
#include <mman.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char test[4096];

char test2[4096];

int sum(int n)
{
	if (n)
		return n + sum(n-1);
	return 0;
}

int main(int argc, char *argv[])
{
	printf("Welcome to test. my pid %d\n", getpid());
	
	int i = 0;
	while(i < argc){
		printf("%s ", argv[i]);
		i++;
	}
	printf("I will do some test and exit\n");	
	while(1);
	
	if (brk(0)) {
		printf("brk failed!\n");
	}

	printf("----start brk/sbrk test----\n");

	char *p = sbrk(0);

	char *new = sbrk(4);
	*new = 0;
	printf("brk %x alloc addr %x\n", p, new);

	p = sbrk(0);
	printf("brk %x\n", p);

	printf("----start malloc test----\n");

	char *a = malloc(16);
	//memset(a, 0, 16);

	char *b = malloc(128);
	//memset(b, 0, 128);

	char *c = malloc(4096);
	//memset(c, 0, 4096);

	printf("a:%x b:%x c:%x\n", a, b, c);

	free(b);
	free(c);
	free(a);
	
	a = malloc(16);
	memset(a, 0, 16);

	b = malloc(256);
	memset(b, 0, 16);
	free(a);
	
	c = malloc(4096);
	memset(c, 0, 4096);
	free(c);
	free(b);

	char *array = calloc(10, 10);
	if (!array)
		printf("calloc failed!\n");

	for (i = 0; i < 10*10; i++) {	
		array[i] = i;
		//printf("%d", i);
	}
	printf("calloc %x", array);

	char *table[10];
	for (i = 0; i < 10; i++) {
		table[i] = malloc(i*32);
		//printf("%x->", table[i]);
	}

	for (i = 0; i < 10; i++) {
		table[i] = realloc(table[i], i*64);
		//printf("%x->", table[i]);
	}

	for (i = 0; i < 10; i++) {
		free(table[i]);
	}

	char *str = malloc(12);
	strcpy(str, "hello,world");
	printf(str);

	str = realloc(str, 32);
	printf(str);
	strcat(str, "I am Book OS\n");
	printf(str);

	char *mm = realloc(NULL, 32);
	printf("realloc addr %x size %d\n", mm, malloc_usable_size(mm));

	realloc(mm, 0);

	mm = malloc(128);
	printf("realloc addr %x size %d\n", mm, malloc_usable_size(mm));

	mm = realloc(mm, 64);
	printf("realloc addr %x size %d\n", mm, malloc_usable_size(mm));

	//memory_state();
	
	char *maped = mmap(0, 4096, 0, 0);

	if(maped == (void *)-1) {
		printf("mmap failed!\n");
		exit(-1);
	}
	
	printf("mmap sucess!\n");
	
	*maped = 0xfa;
	printf("mmap %x %x.", maped, *maped);

	return 0;
}
