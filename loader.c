#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef void (*func)();

// ELFファイルか判定
static int isElf(char *head) {
	if(head[EI_MAG0] != ELFMAG0 || head[EI_MAG1] != ELFMAG1 || head[EI_MAG2] != ELFMAG2 || head[EI_MAG3] != ELFMAG3) return 0;
	return 1;
}

// num番目のセクションヘッダを所得
static Elf32_Shdr *getShdr(Elf32_Ehdr *ehdr, int num) {
	Elf32_Shdr *shdr;
	if(num < 0 || num >= ehdr->e_shnum) {
		fprintf(stderr, "cannnot find shdr %d\n", num);
		return NULL;
	}

	shdr = (Elf32_Shdr *)((char *)ehdr + ehdr->e_shoff + ehdr->e_shentsize * num);
	return shdr;
}

// セクションを名前で検索
static Elf32_Shdr *searchShdr(Elf32_Ehdr *ehdr, char *name) {
	int i;
	Elf32_Shdr *shdr, *ndx;

	ndx = getShdr(ehdr, ehdr->e_shstrndx);
	for(i = 0; i < ehdr->e_shnum; i++) {
		shdr = getShdr(ehdr, i);
		if(!strcmp((char *)ehdr + ndx->sh_offset + shdr->sh_name, name)) {
			return shdr;
		}
	}

	fprintf(stderr, "cannnot find shdr %s\n", name);
	return NULL;
}

static func load(char *head) {
	int i;
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdr;
	Elf32_Shdr *shdr;
	func f;

	ehdr = (Elf32_Ehdr *)head;
	if(!isElf(ehdr)) {
		fprintf(stderr, "This file is not ELF\n");
		return NULL;
	}

	for(i = 0; i < ehdr->e_phnum; i++) {
		phdr = (Elf32_Phdr *)(head + ehdr->e_phoff + ehdr->e_phentsize * i);
		
		// ローダのテキスト領域の先頭にロード(リンカスクリプトで空きを作っておく)
		if(phdr->p_type == PT_LOAD) {
			memcpy((char *)phdr->p_vaddr, head + phdr->p_offset, phdr->p_filesz);
		}
	}

	// BSS領域をイニシャライズ
	shdr = searchShdr(ehdr, ".bss");
	if(shdr) {
		printf("clear BSS: 0x%08x\n", shdr->sh_addr);
		memset((char *)shdr->sh_addr, 0, shdr->sh_size);
	}
	
	f = (func)ehdr->e_entry;
	printf("Entry Point: 0x%08x\n", (int)f);

	return f;
}

#define ARGV

int main(int argc, char *argv[]) {
	int fd;
	struct stat sb;
	char *head;
	func f;
	char filename[128];

#ifdef ARGV
	char **stackargs;
#endif

	strcpy(filename, argv[1]);

	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		fprintf(stderr, "cannnot open file %s\n", filename);
		return -1;
	}
	fstat(fd, &sb);
	head = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	f = load(head);
	if(f == NULL) {
		fprintf(stderr, "failed to load\n");
		return -1;
	}
	close(fd);

	printf("lets go");

	// 引数の準備
#ifndef ARGV
	asm volatile("pushl $0");
	asm volatile("pushl $0");
	asm volatile("pushl %0" :: "m"(filename));
	asm volatile("pushl $1");
#else
	argv[0] = (char *)(argc - 1);
	stackargs = &argv[0];
	asm volatile("movl %0, %%esp" :: "m"(stackargs));
#endif

	// エントリポイントにジャンプ
	asm volatile("jmp *%0" :: "m"(f));

	exit(0);

	return 0;
}
