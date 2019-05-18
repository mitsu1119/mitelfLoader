#include <stdio.h>
#include <stdlib.h>

int global_bss;
int global_data = 10;

void print() {
	printf("----------------------------------------------\n");
	printf("call print()\n");
	printf("----------------------------------------------\n");
}

int main(int argc, char *argv[]) {
	int local = 0;

	global_bss = 11;

	printf("----------------------------------------------\n");
	printf("argc        == %d\n", argc);
	for(local = 0; local < argc; local++) printf("argv[%d] : %s\n", local, argv[local]);
	printf("----------------------------------------------\n");
	printf("global_bss  == %d\n", global_bss);
	printf("global_data == %d\n", global_data);
	printf("local       == %d\n", local);
	printf("----------------------------------------------\n");

	print();

	exit(0);
}
