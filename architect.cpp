#include <cstring>
#include <fstream>
#include <unistd.h>
#include <cstdio>
#include <elf.h>
#include <sys/mman.h>

#if ARCH == 32
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Shdr Elf32_Shdr
#else
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#endif

char* getString(char* string_table, int index)
{
    int i = 0;
    int offset = 0;
    while (i < index)
    {
        offset += strlen(string_table) + 1;
        i++;
    }

    return &string_table[offset];
}

int main(int argc, char* argv[])
{
    if (argc == 0)
    {
        fprintf(stderr, "Called without 0th argument - HOW?\n");
        return 1;
    }

    // Open this file for reading
    std::ifstream self(argv[0]);
    Elf_Ehdr elfHeader;
    self.read(reinterpret_cast<char*>(&elfHeader), sizeof(elfHeader)); // Look, I `know` this is cursed

    if (elfHeader.e_shoff == 0 || elfHeader.e_shnum == 0)
    {
        fprintf(stderr, "No ELF sections found\n");
    }

    self.seekg(elfHeader.e_shoff);

    Elf_Shdr* section_headers = reinterpret_cast<Elf_Shdr*>(malloc(elfHeader.e_shnum * elfHeader.e_shentsize));
    self.read(reinterpret_cast<char*>(section_headers), elfHeader.e_shnum * elfHeader.e_shentsize);

    char* string_table = reinterpret_cast<char*>(malloc(section_headers[elfHeader.e_shstrndx].sh_size));
    self.seekg(section_headers[elfHeader.e_shstrndx].sh_offset);
    self.read(reinterpret_cast<char*>(string_table), section_headers[elfHeader.e_shstrndx].sh_size);

    int payload_section = -1;
    for (int i = 0; i < elfHeader.e_shnum; i++)
    {
        char* section_name = getString(string_table, section_headers[i].sh_name);
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

    mmap()

    const pid_t pid = getpid();

    free(section_headers);

    return 0;
}