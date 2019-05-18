#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/mman.h>

static int isElf(char *head) {
	if(head[EI_MAG0] != ELFMAG0 || head[EI_MAG1] != ELFMAG1 || head[EI_MAG2] != ELFMAG2 || head[EI_MAG3] != ELFMAG3) return 0;
	return 1;
}

static int chflg(char *head) {
	int i;
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdr;

	ehdr = (Elf32_Ehdr *)head;
	if(!isElf(ehdr)) {
		fprintf(stderr, "This file is not ELF\n");
		return -1;
	}

	for(i = 0; i < ehdr->e_phnum; i++) {
		phdr = (Elf32_Phdr *)((char *)ehdr + ehdr->e_phoff + ehdr->e_phentsize * i);
		phdr->p_flags = PF_X | PF_W | PF_R;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	int fd;
	struct stat sb;
	char *head;

	fd = open(argv[1], O_RDWR);
	if(fd < 0) {
		fprintf(stderr, "cannnot open file %s.\n", argv[1]);
		return -1;
	}

	fstat(fd, &sb);
	head = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(chflg(head) < 0) {
		fprintf(stderr, "fail to change flags.\n");
	}
	close(fd);

	return 0;
}

