#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#define MAX_CHUNK_NAME_LENGTH 64

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "usage: %s <destination> <prefix> <chunk size (K)> <number of chunks>\n", argv[0]);
        return 1;
    }

    const char *destination_filename = argv[1];
    const char *prefix = argv[2];
    long long chunk_size_kb = strtoll(argv[3], NULL, 10); 
    long long num_chunks = strtoll(argv[4], NULL, 10);   

    if (chunk_size_kb <= 0 || num_chunks <= 0) {
        fprintf(stderr, "Error: Invalid chunk size or number of chunks.\n");
        return 1;
    }

    long long chunk_size_bytes = chunk_size_kb * 1024; 
    
    FILE *destination_file = fopen(destination_filename, "wb");
    if (!destination_file) {
        fprintf(stderr, "Error: Could not open destination file: %s\n", destination_filename);
        return 1;
    }

    unsigned char *buffer = (unsigned char *)malloc(chunk_size_bytes);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(destination_file);
        return 1;
    }

    int missing_chunk = 0;
    int incorrect_chunk_size = 0;

    for (long long i = 0; i < num_chunks; ++i) {
        char chunk_filename[MAX_CHUNK_NAME_LENGTH];
        snprintf(chunk_filename, sizeof(chunk_filename), "%s.%032lld", prefix, i);

        printf("putting %s\n", chunk_filename);

        FILE *chunk_file = fopen(chunk_filename, "rb");
        if (chunk_file) {
            size_t bytes_read = fread(buffer, 1, chunk_size_bytes, chunk_file);
            fwrite(buffer, 1, bytes_read, destination_file);
            fclose(chunk_file); 

            if (i < num_chunks - 1 && (long long)bytes_read != chunk_size_bytes) {
                fprintf(stderr, "Warning: Incorrect chunk size for file: %s\n", chunk_filename);
                incorrect_chunk_size = 1;
            }
        } else {
            fprintf(stderr, "Error: Could not open chunk file: %s\n", chunk_filename);
            missing_chunk = 1;
        }
    }

    fclose(destination_file);
    free(buffer);             

    if (missing_chunk) {
        fprintf(stderr, "Error: One or more chunk files are missing.\n");
        remove(destination_filename); 
        return 1;
    }

    if (incorrect_chunk_size) {
        fprintf(stderr, "Warning: One or more chunk files have incorrect size.\n");
    }

    printf("done... [%lld] chunks produced for %s\n", num_chunks, destination_filename);

    return 0;
}
