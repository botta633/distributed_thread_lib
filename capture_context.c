#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include "capture_context.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h> // for mkdir
#include <dirent.h>   // for opendir and readdir
#include <fcntl.h>    // for readlink
#include <errno.h>    // for errno

// Forward declarations
static int capture_memory(int tid, const char *dir_name);
static int capture_open_fds(int tid, const char *dir_name);
static void create_zip(int tid, char dir_name[256]);
static int copy_with_sudo(const char *src, const char *dst);

// Directory management functions
// Create a directory for the thread dump
static int create_dump_directory(int tid, char *dir_name, size_t dir_name_size)
{
    if (dir_name == NULL || dir_name_size < 1)
    {
        return -1;
    }

    // Create directory name
    snprintf(dir_name, dir_name_size, "./thread_%d_dump", tid);

    // Remove if exists
    char rm_command[256];
    snprintf(rm_command, sizeof(rm_command), "rm -rf %s", dir_name);
    system(rm_command);

    // Create new directory
    if (mkdir(dir_name, 0755) == -1)
    {
        perror("mkdir failed");
        return -1;
    }

    // Verify directory is writable
    if (access(dir_name, W_OK) == -1)
    {
        perror("directory not writable");
        return -1;
    }

    return 0;
}

// Main orchestrator function
void capture_context(int tid)
{
    // Create directory for this capture
    char dir_name[256];
    printf("Creating dump directory for pid: %d tid: %d\n", getpid(), tid);
    if (create_dump_directory(tid, dir_name, sizeof(dir_name)) != 0)
    {
        fprintf(stderr, "Failed to create dump directory\n");
        exit(1);
    }

    // Capture memory pages
    if (capture_memory(tid, dir_name) != 0)
    {
        fprintf(stderr, "Failed to capture memory\n");
        exit(1);
    }

    // Capture open file descriptors
    if (capture_open_fds(getpid(), dir_name) != 0)
    {
        fprintf(stderr, "Failed to capture FDs\n");
        exit(1);
    }

    // Create zip and cleanup
    create_zip(tid, dir_name);
}

// Memory capture functions
// Open the proc maps file for the thread
static FILE *open_proc_maps(int tid)
{
    char proc_maps_path[256];
    snprintf(proc_maps_path, sizeof(proc_maps_path),
             "/proc/%d/task/%d/maps", getpid(), tid);

    FILE *fp = fopen(proc_maps_path, "r");
    if (fp == NULL)
    {
        perror("Failed to open proc maps");
        return NULL;
    }

    return fp;
}

// Memory capture function
static int capture_memory(int tid, const char *dir_name)
{
    FILE *fp = open_proc_maps(tid);
    if (fp == NULL)
    {
        return -1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        unsigned long start, end;
        char perms[5];
        if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) == 3)
        {
            if (perms[0] == 'r')
            {
                size_t range_size = end - start;
                void *mem = (void *)start;

                char filename[512];
                snprintf(filename, sizeof(filename),
                         "%s/memory_dump_%lx_%lx.bin", dir_name, start, end);

                FILE *memfile = fopen(filename, "wb");
                if (memfile != NULL)
                {
                    if (fwrite(mem, 1, range_size, memfile) != range_size)
                    {
                        fprintf(stderr, "Warning: Could not write full range %lx-%lx\n",
                                start, end);
                    }
                    fclose(memfile);
                }
            }
        }
    }

    fclose(fp);
    free(line);
    return 0;
}

// File descriptor capture functions
// Capture the open file descriptors for the thread
static int capture_open_fds(int tid, const char *dir_name)
{
    char proc_fd_path[256];
    sprintf(proc_fd_path, "/proc/%d/task/%d/fd", getpid(), tid);

    // open the proc fd path
    DIR *dir = opendir(proc_fd_path);
    if (dir == NULL)
    {
        perror("Failed to open proc fd");
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        //exclude the fd 0. 1. 2
        if (strcmp(entry->d_name, "0") == 0 || strcmp(entry->d_name, "1") == 0 || strcmp(entry->d_name, "2") == 0)
        {
            continue;
        }

        char fd_path[256];
        sprintf(fd_path, "%s/%s", proc_fd_path, entry->d_name);

        char link_path[256];
        ssize_t len = readlink(fd_path, link_path, sizeof(link_path) - 1);
        if (len != -1)
        {
            link_path[len] = '\0';
        }
        char filename[512];
        sprintf(filename, "%s/open_fd_%s", dir_name, entry->d_name);
        //exculde the fd that refers to the current open proc dir
        if (strstr(link_path, "/proc/") != NULL)
        {
            continue;
        }
        char cp_command[512];
        sprintf(cp_command, "sudo cp %s %s", link_path, filename);
        int status = system(cp_command);
        if (status != 0)
        {
            printf("Failed to copy FD %s\n", entry->d_name);
        }
    }

    closedir(dir);
    return 0;
}

// Utility functions
// Create a zip file from the thread dump directory
static void create_zip(int tid, char dir_name[256])
{
    // Create zip file
    char zip_command[512];

    sprintf(zip_command, "zip -r thread_%d_dump.zip %s", tid, dir_name);
    system(zip_command);

    // remove the directory
    char rm_command[256];
    snprintf(rm_command, sizeof(rm_command), "rm -rf %s", dir_name);
    system(rm_command);
}

static int copy_with_sudo(const char *src, const char *dst)
{
    char command[1024];
    snprintf(command, sizeof(command), "sudo cp %s %s 2>/dev/null", src, dst);
    return system(command);
}