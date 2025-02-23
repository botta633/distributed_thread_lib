#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include "capture_context.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>  // for mkdir

/**
 * Captures the memory maps for a thread with tid 
 * and parses the output to a char pointer
 * 
 * @param buffer: the buffer to store the output
 * @param mem: the memory address to search for
 * @param tid: the thread id
 */

void capture_context(int tid)
{

    char dir_name[256];
    sprintf(dir_name, "./thread_%d_dump", tid);
    char rm_command[256];
    sprintf(rm_command, "rm -rf %s", dir_name);
    system(rm_command);
    
    if (mkdir(dir_name, 0755) == -1) {
        perror("mkdir failed");
        exit(1);
    }
    
    if(access(dir_name, W_OK) == -1) {
        perror("directory not writable");
        exit(1);
    }

    char proc_maps_path[256];
    sprintf(proc_maps_path, "/proc/%d/task/%d/maps", getpid(), tid);
    FILE *fp = fopen(proc_maps_path, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    // read memory in every range and write a file with every page of memory to serialize it
    // parse the output

//TODO-> Test the director creation
// test content of the memory
//Work on restoration from the remote machine
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        // Parse memory range from line
        unsigned long start, end;
        char perms[5];
        if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) == 3)
        {
            // Only read if we have read permission
            if (perms[0] == 'r')
            {

                size_t range_size = end - start;
                void *mem = (void *)start;

                // Create a filename inside the thread directory
                char filename[512];
                sprintf(filename, "%s/memory_dump_%lx_%lx.bin", dir_name, start, end);

                // Open file for writing
                FILE *memfile = fopen(filename, "wb");
                if (memfile != NULL)
                {
                    // Try to read and write the memory range
                    if (fwrite(mem, 1, range_size, memfile) != range_size)
                    {
                        fprintf(stderr, "Warning: Could not write full range %lx-%lx\n", start, end);
                    }
                    fclose(memfile);
                }
            }
        }
    }
    
    // Create zip file
    char zip_command[512];

    sprintf(zip_command, "zip -r thread_%d_dump.zip %s", tid, dir_name);
    system(zip_command);

    // remove the directory
    sprintf(rm_command, "rm -rf %s", dir_name);
    system(rm_command);
   
    
    fclose(fp);
    free(line);
}
