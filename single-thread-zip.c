#include <stdio.h>
#include <stdlib.h>

void compressFile(FILE *fp) {
    int count;
    char current, previous;

    if ((previous = fgetc(fp)) != EOF) {
        count = 1;
        while ((current = fgetc(fp)) != EOF) {
            if (current == previous) {
                count++;
            } else {
                fwrite(&count, sizeof(int), 1, stdout);
                fwrite(&previous, sizeof(char), 1, stdout);
                previous = current;
                count = 1;
            }
        }
        fwrite(&count, sizeof(int), 1, stdout);
        fwrite(&previous, sizeof(char), 1, stdout);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            perror("wzip");
            return 1;
        }
        compressFile(fp);
        fclose(fp);
    }

    return 0;
}
