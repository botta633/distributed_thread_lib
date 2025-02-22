#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include "capture_context.h"
#include <string.h>
#include <stdio.h>

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
    char proc_maps_path[256];
    sprintf(proc_maps_path, "/proc/%d/task/%d/maps", getpid(), tid);
    
    FILE *fp = fopen(proc_maps_path, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    // read memory in every range and write a file with every page of memory to serialize it
    // parse the output

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
            printf("Came here\n");
            // Only read if we have read permission
            if (perms[0] == 'r')
            {
                size_t range_size = end - start;
                void *mem = (void *)start;

                // Create a unique filename for this range
                char filename[256];
                sprintf(filename, "memory_dump_%d_%lx_%lx.bin", tid, start, end);

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
    pclose(fp);
    free(line);
}
