#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#define MAX_NAME 8192

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <destination> <prefix>\n", argv[0]);
        return 1;
    }

    const char *destination_filename = argv[1];
    const char *prefix = argv[2];
    char meta_filename[MAX_NAME];
    snprintf(meta_filename, sizeof(meta_filename), "%s.meta", prefix);

    FILE *meta_file = fopen(meta_filename, "r");
    if (!meta_file) {
        fprintf(stderr, "Error: Could not open meta file: %s\n", meta_filename);
        return 1;
    }

    int num_chunks, chunk_size;
    if (fscanf(meta_file, "%d\n%d\n", &num_chunks, &chunk_size) != 2) {
        fprintf(stderr, "Error: Invalid meta file format.\n");
        fclose(meta_file);
        return 1;
    }
    fclose(meta_file);

    FILE *destination_file = fopen(destination_filename, "wb");
    if (!destination_file) {
        fprintf(stderr, "Error: Could not open destination file: %s\n", destination_filename);
        return 1;
    }

    unsigned char *buffer = (unsigned char *)malloc(chunk_size);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(destination_file);
        return 1;
    }

    int missing_chunk = 0;
    int incorrect_chunk_size = 0;

    for (int i = 0; i < num_chunks; ++i) {
        char chunk_filename[MAX_NAME];
        snprintf(chunk_filename, sizeof(chunk_filename), "%s.%032d", prefix, i);

        printf("putting %s\n", chunk_filename);

        FILE *chunk_file = fopen(chunk_filename, "rb");
        if (chunk_file) {
            size_t bytes_read = fread(buffer, 1, chunk_size, chunk_file);
            fwrite(buffer, 1, bytes_read, destination_file);
            fclose(chunk_file);

            if (i < num_chunks - 1 && (int)bytes_read != chunk_size) {
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

    printf("done... [%d] chunks produced for %s\n", num_chunks, destination_filename);

    return 0;
}
