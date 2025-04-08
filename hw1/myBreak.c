#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h> 

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s <source> <prefix> <chunk size (K)>\n", argv[0]);
        return 1;
    }

    const char *source_filename = argv[1];
    const char *prefix = argv[2];
    long long chunk_size_kb;
    char *endptr;

    chunk_size_kb = strtoll(argv[3], &endptr, 10);
    if (*endptr != '\0' || chunk_size_kb <= 0) {
        fprintf(stderr, "Error: Invalid chunk size provided.\n");
        return 1;
    }

    long long chunk_size_bytes = chunk_size_kb * 1024;
    int source_fd = open(source_filename, O_RDONLY);
    if (source_fd == -1) {
        fprintf(stderr, "Error: Could not open source file: %s (%s)\n", source_filename, strerror(errno));
        return 1;
    }

    // Determine the size of the source file using fstat
    struct stat stat_buf;
    if (fstat(source_fd, &stat_buf) == -1) {
        fprintf(stderr, "Error getting file size for: %s (%s)\n", source_filename, strerror(errno));
        close(source_fd);
        return 1;
    }
    long long source_file_size = (long long)stat_buf.st_size;

    long long num_chunks = (source_file_size + chunk_size_bytes - 1) / chunk_size_bytes;
    unsigned char *buffer = (unsigned char *)malloc(chunk_size_bytes);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        close(source_fd);
        return 1;
    }

    for (long long i = 0; i < num_chunks; ++i) {
        char output_filename[512]; // Adjust size as needed
        snprintf(output_filename, sizeof(output_filename), "%s.%032lld", prefix, i);

        printf("starting %s\n", output_filename);

        int output_fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            fprintf(stderr, "Error: Could not open output file: %s (%s)\n", output_filename, strerror(errno));
            close(source_fd);
            free(buffer);
            return 1;
        }

        size_t bytes_to_read = chunk_size_bytes;
        if (i == num_chunks - 1) {
            bytes_to_read = source_file_size - i * chunk_size_bytes;
        }

        ssize_t bytes_read = read(source_fd, buffer, bytes_to_read);
        if (bytes_read > 0) {
            ssize_t bytes_written = write(output_fd, buffer, bytes_read);
            if (bytes_written != bytes_read) {
                fprintf(stderr, "Error writing to output file: %s (%s)\n", output_filename, strerror(errno));
                close(output_fd);
                close(source_fd);
                free(buffer);
                return 1;
            }
        } else if (bytes_read == -1) {
            fprintf(stderr, "Error reading source file: %s (%s)\n", source_filename, strerror(errno));
            close(output_fd);
            close(source_fd);
            free(buffer);
            return 1;
        }

        close(output_fd);
    }

    close(source_fd);
    free(buffer);
    printf("done... [%lld] chunks produced for %s\n", num_chunks, source_filename);

    return 0;
}
