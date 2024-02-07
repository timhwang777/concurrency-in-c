#include <stdio.h>
#include <stdlib.h>

void decompressFile(FILE *fp) {
    int count;
    char character;

    while (fread(&count, sizeof(int), 1, fp) == 1) {
        fread(&character, sizeof(char), 1, fp);
        for (int i = 0; i < count; i++) {
            printf("%c", character);
        }
    }

    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            perror("wunzip");
            return 1;
        }
        decompressFile(fp);
        fclose(fp);
    }

    return 0;
}
