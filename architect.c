#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <elf.h>
#include <sys/mman.h>

#if ARCH == 32
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Shdr Elf32_Shdr
#else
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#endif

int main(int argc, char* argv[])
{
    if (argc == 0)
    {
        fprintf(stderr, "Called without 0th argument - HOW?\n");
        return 1;
    }

    // Open this file for reading
    FILE* self = fopen(argv[0], "rb");
    Elf_Ehdr elfHeader;
    fread(&elfHeader, sizeof(elfHeader), 1, self);

    if (elfHeader.e_shoff == 0 || elfHeader.e_shnum == 0)
    {
        fprintf(stderr, "No ELF sections found\n");
    }

    Elf_Shdr* section_headers = malloc(elfHeader.e_shnum * elfHeader.e_shentsize);
    fseek(self, elfHeader.e_shoff, SEEK_SET);
    fread(section_headers, elfHeader.e_shentsize, elfHeader.e_shnum, self);

    char* string_table = malloc(section_headers[elfHeader.e_shstrndx].sh_size);
    fseek(self, section_headers[elfHeader.e_shstrndx].sh_offset, SEEK_SET);
    fread(string_table, section_headers[elfHeader.e_shstrndx].sh_size, 1, self);

    int payload_section = -1;
    for (int i = 0; i < elfHeader.e_shnum; i++)
    {
        char* section_name = &string_table[section_headers[i].sh_name];
        if (strcmp(section_name, ".payload") == 0)
        {
            payload_section = i;
            break;
        }
    }

    if (payload_section == -1)
    {
        fprintf(stderr, "No payload section found\n");
        return 1;
    }

    //mmap()

    const pid_t pid = getpid();

    free(section_headers);
    fclose(self);

    return 0;
}