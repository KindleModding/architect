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
#include <sys/types.h>
#include <sys/wait.h>

#if ARCH == 32
typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Shdr Elf_Shdr;
typedef Elf32_Xword Elf_Xword;
#else
typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Shdr Elf_Shdr;
typedef Elf64_Xword Elf_Xword;
#endif

int main(int argc, char* argv[], char *envp[])
{
    if (argc == 0)
    {
        fprintf(stderr, "ERROR: Called improperly.\n");
        return 1;
    }

    // Open this file for reading
    FILE* self = fopen("/proc/self/exe", "rb");
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

    char* filepath = strdup("/tmp/architect_XXXXXX");

    int fd = mkostemp(filepath, O_EXCL);
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
    
    char** final_argv = malloc(sizeof(char*) * (argc - 1 + 4 + 1)); // argv[0] is handled separetely, add the 4 args we need for that, and also we need to NULL-terminate the char* array
    final_argv[0] = interpreter;
    final_argv[1] = "--argv0";
    final_argv[2] = argv[0];
    final_argv[3] = filepath;
    memcpy(&final_argv[4], &argv[1], sizeof(char*) * (argc-1)); // We copy from the 1st element to the last
    final_argv[argc - 1 + 4] = NULL;

    pid_t pid = fork();
    int status;
    switch (pid)
    {
        case -1:
            fprintf(stderr, "Fork error! (%i)", errno);
            break;
        case 0:
            execve(interpreter, final_argv, envp);
            break;
        default:
            waitpid(pid, &status, 0);
            remove(filepath);
            free(filepath);
            return status;
            break;
    }

    return 1;
}