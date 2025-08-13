#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <elf.h>
#include <sys/syscall.h>
#include <linux/memfd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#if ARCH == 32
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Shdr Elf32_Shdr
#define Elf_Xword Elf32_Xword
#else
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#define Elf_Xword Elf64_Xword
#endif

int main(int argc, char* argv[], char *envp[])
{
    if (argc == 0)
    {
        fprintf(stderr, "ERROR: Called improperly.\n");
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

    int sf_payload_section = -1;
    int hf_payload_section = -1;
    for (int i = 0; i < elfHeader.e_shnum; i++)
    {
        char* section_name = string_table + section_headers[i].sh_name;
        if (strcmp(section_name, ".sf_payload") == 0)
        {
            sf_payload_section = i;
        }
        if (strcmp(section_name, ".hf_payload") == 0)
        {
            hf_payload_section = i;
        }
    }

    if (sf_payload_section == -1)
    {
        fprintf(stderr, "No sf_payload section found\n");
        return 1;
    }

    // Check our architecture
    int payload_section = sf_payload_section;
    char* interpreter = "/lib/ld-linux.so.3";
    if (access("/lib/ld-linux-armhf.so.3", R_OK) == 0)
    {
        interpreter = "/lib/ld-linux-armhf.so.3";

        if (hf_payload_section != -1)
        {
            payload_section = hf_payload_section;
        }
    }

    char* filename = malloc(sizeof(char) * 21);
    memcpy(filename, "/tmp/architect_XXXXXX", sizeof(char) * 21);

    int fd = mkostemp(filename, O_EXCL);
    FILE* payload_file = fdopen(fd, "wb");
    fchmod(fd, S_IRUSR|S_IWUSR|S_IXUSR|S_IXGRP|S_IXOTH);
    
    fseek(self, section_headers[payload_section].sh_offset, SEEK_SET);
    for (Elf_Xword i=0; i < section_headers[payload_section].sh_size; i++)
    {
        fputc(fgetc(self), payload_file);
    }

    free(section_headers);
    free(string_table);
    fclose(self);
    fclose(payload_file);
    
    char** final_argv = malloc(sizeof(char*) * (argc + 3));
    final_argv[0] = interpreter;
    final_argv[1] = filename;
    memcpy(&final_argv[2], argv, sizeof(char*) * (argc + 1));
    execve(interpreter, final_argv, envp);

    fprintf(stderr, "Fatal error - %i\n", errno);
    
    return 1;
}