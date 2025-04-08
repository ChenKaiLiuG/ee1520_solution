#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h> 

int file_exists(const char *filename) {
    return (access(filename, F_OK) == 0);
}

uintmax_t file_size(const char *filename) {
    struct stat stat_buf;
    if (stat(filename, &stat_buf) == 0) 
        return (uintmax_t)stat_buf.st_size;
        
    return (uintmax_t)-1;
}

int main(int argc, char *argv[]) {
    if (argc != 5) 
    {
        fprintf(stderr, "usage: %s <destination> <prefix> <chunk size (K)> <number of chunks>\n", argv[0]);
        return 1;
    }

    const char *destination_filename = argv[1];
    const char *prefix = argv[2];
    long long expected_chunk_size_kb;
    long long num_chunks;
    char *endptr;

    expected_chunk_size_kb = strtoll(argv[3], &endptr, 10);
    if (*endptr != '\0' || expected_chunk_size_kb <= 0) 
    {
        fprintf(stderr, "Error: Invalid chunk size provided.\n");
        return 1;
    }

    num_chunks = strtoll(argv[4], &endptr, 10);
    if (*endptr != '\0' || num_chunks <= 0) 
    {
        fprintf(stderr, "Error: Invalid number of chunks provided.\n");
        return 1;
    }

    long long expected_chunk_size_bytes = expected_chunk_size_kb * 1024;
    int destination_fd = open(destination_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destination_fd == -1) 
    {
        fprintf(stderr, "Error: Could not open/create destination file: %s (%s)\n", destination_filename, strerror(errno));
        return 1;
    }

    unsigned char *buffer = (unsigned char *)malloc(expected_chunk_size_bytes);
    if (!buffer) 
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        close(destination_fd);
        remove(destination_filename);
        return 1;
    }

    int missing_chunk = 0;
    int incorrect_chunk_size = 0;

    for (long long i = 0; i < num_chunks; ++i) 
    {
        char chunk_filename[512]; 
        snprintf(chunk_filename, sizeof(chunk_filename), "%s.%032lld", prefix, i);

        printf("putting %s\n", chunk_filename);
        int chunk_fd = open(chunk_filename, O_RDONLY);
        if (chunk_fd != -1) 
        {
            ssize_t bytes_read = read(chunk_fd, buffer, expected_chunk_size_bytes);
            if (bytes_read > 0) 
            {
                ssize_t bytes_written = write(destination_fd, buffer, bytes_read);
                if (bytes_written != bytes_read) 
                {
                    fprintf(stderr, "Error writing to destination file from chunk: %s (%s)\n", chunk_filename, strerror(errno));
                    close(chunk_fd);
                    close(destination_fd);
                    free(buffer);
                    remove(destination_filename);
                    return 1;
                }
            } 
            else if (bytes_read == -1) 
            {
                fprintf(stderr, "Error reading chunk file: %s (%s)\n", chunk_filename, strerror(errno));
                missing_chunk = 1;
            }
            close(chunk_fd);

            if (i < num_chunks - 1 && bytes_read != expected_chunk_size_bytes) 
            {
                fprintf(stderr, "Warning: Incorrect chunk size for file: %s. Expected: %lld, Actual: %zd\n", chunk_filename, expected_chunk_size_bytes, bytes_read);
                incorrect_chunk_size = 1;
            }
        } 
        else 
        {
            fprintf(stderr, "Error: Could not open chunk file: %s (%s)\n", chunk_filename, strerror(errno));
            missing_chunk = 1;
        }
    }

    close(destination_fd);
    free(buffer);

    if (missing_chunk) 
    {
        fprintf(stderr, "Error: One or more chunk files are missing. Reassembly failed.\n");
        remove(destination_filename);
        return 1;
    }

    if (incorrect_chunk_size) 
    {
        fprintf(stderr, "Warning: One or more chunk files have an incorrect size. The reassembled file might be incomplete or corrupted.\n");
        return 1;
    }

    printf("done... [%lld] chunks produced for %s\n", num_chunks, destination_filename);

    return 0;
}
