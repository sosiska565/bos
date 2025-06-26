#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned int nfiles;
} initrd_header_t;

typedef struct {
    unsigned char magic;
    char name[64];
    unsigned int offset;
    unsigned int length;
} initrd_file_header_t;

int main(int argc, char **argv) {
    if (argc < 2 || (argc - 2) % 2 != 0) {
        fprintf(stderr, "Usage: %s <output_file> [file_path internal_name]...\n", argv[0]);
        return 1;
    }

    char *output_path = argv[1];
    int n_files = (argc - 2) / 2;

    initrd_file_header_t *headers = malloc(sizeof(initrd_file_header_t) * n_files);
    if (!headers) {
        perror("malloc for headers failed");
        return 1;
    }

    printf("Creating initrd with %d files.\n", n_files);

    unsigned int current_offset = sizeof(initrd_header_t) + sizeof(initrd_file_header_t) * n_files;

    for (int i = 0; i < n_files; i++) {
        char *file_path = argv[2 + i * 2];
        char *internal_name = argv[2 + i * 2 + 1];

        strncpy(headers[i].name, internal_name, sizeof(headers[i].name) - 1);
        headers[i].name[sizeof(headers[i].name) - 1] = '\0';
        headers[i].magic = 0xBF;

        FILE *f = fopen(file_path, "rb");
        if (!f) {
            fprintf(stderr, "Could not open file: %s\n", file_path);
            free(headers);
            return 1;
        }

        fseek(f, 0, SEEK_END);
        headers[i].length = ftell(f);
        fclose(f);

        headers[i].offset = current_offset;
        current_offset += headers[i].length;

        printf("  - %s (%s) -> %u bytes at offset %u\n", file_path, internal_name, headers[i].length, headers[i].offset);
    }

    FILE *out = fopen(output_path, "wb");
    if (!out) {
        fprintf(stderr, "Could not create output file: %s\n", output_path);
        free(headers);
        return 1;
    }

    initrd_header_t main_header = { .nfiles = n_files };
    fwrite(&main_header, sizeof(main_header), 1, out);
    fwrite(headers, sizeof(initrd_file_header_t), n_files, out);

    for (int i = 0; i < n_files; i++) {
        char *file_path = argv[2 + i * 2];
        FILE *f = fopen(file_path, "rb");
        char *buffer = malloc(headers[i].length);
        if (!buffer) {
            perror("malloc for file buffer failed");
            fclose(out);
            free(headers);
            return 1;
        }
        fread(buffer, 1, headers[i].length, f);
        fwrite(buffer, 1, headers[i].length, out);
        fclose(f);
        free(buffer);
    }

    fclose(out);
    free(headers);

    printf("Successfully created %s\n", output_path);
    return 0;
}
