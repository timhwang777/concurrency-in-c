#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>

/*
    File structure
*/
typedef struct {
    char* data;
    size_t size;
} file_t;

/*
    Thread arguments structure
*/
typedef struct {
    int threadID;
    char* data;
    size_t segSize;
    char* outputFilename;
    char* outputBuffer;
    size_t outputSize;
} thread_t;

/*
    Compression function for each thread
*/
void* compress(void* arg) {
    thread_t* thread = (thread_t*) arg;

    char* data = thread->data;
    size_t segSize = thread->segSize;
    int count = 1;
    char current, previous;

    // Create the buffer for the current thread's output
    char* outputBuffer = malloc(segSize * (sizeof(int) + sizeof(char)));
    if (!outputBuffer) {
        perror("Failed to allocate memory for output buffer");
        return NULL;
    }
    char* bufferPtr = outputBuffer;

    previous = data[0];
    for (size_t i = 1; i < segSize; ++i) {
        current = data[i];
        if (current == previous) {
            count++;
        } else {
            memcpy(bufferPtr, &count, sizeof(int));
            bufferPtr += sizeof(int);
            memcpy(bufferPtr, &previous, sizeof(char));
            bufferPtr += sizeof(char);
            printf("Thread %d: %d %c\n", thread->threadID, count, previous);

            previous = current;
            count = 1;
        }
    }

    // Write the last sequence
    memcpy(bufferPtr, &count, sizeof(int));
    bufferPtr += sizeof(int);
    memcpy(bufferPtr, &previous, sizeof(char));
    bufferPtr += sizeof(char);
    printf("Thread %d: %d %c\n", thread->threadID, count, previous);

    thread->outputBuffer = outputBuffer;
    thread->outputSize = bufferPtr - outputBuffer;

    return NULL;
}

/*
    Main function: read the input files and create the threads
*/
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "pzip: file1 [file2 ...]\n");
        exit(1);
    }
    
    int numThreads = get_nprocs();
    fprintf(stderr, "Number of threads: %d\n", numThreads);
    pthread_t threads[numThreads];
    thread_t threadArgs[numThreads];

    for(int file = 1 ; file < argc ; file++) {
        int fd = open(argv[file], O_RDONLY);
        printf("\033[0;33mFile: %s\033[0m\n", argv[file]);
        if (fd == -1) {
            perror("pzip: cannot open file");
            exit(1);
        }

        // Get the file basic information, file size
        struct stat st;
        if (fstat(fd, &st) == -1) {
            perror("pzip: cannot stat file");
            exit(1);
        }
        size_t fileSize = st.st_size;
        fprintf(stderr, "File size: %ld\n", fileSize);

        // Map the file to memory
        char* data = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED) {
            perror("pzip: cannot mmap file");
            exit(1);
        }

        size_t bytesPerThread = fileSize / numThreads;
        fprintf(stderr, "Bytes per thread: %ld\n", bytesPerThread);
        // If the file is too small, reduce the number of threads
        if (bytesPerThread == 0) {
            // Ensure at least one thread for non-empty files
            numThreads = fileSize > 0 ? fileSize : 1;
            // At least one byte per thread
            bytesPerThread = 1;
        }

        fprintf(stderr, "Number of thread needed: %d\n", numThreads);

        // Create the output file name
        char outputFilename[256];
        snprintf(outputFilename, sizeof(outputFilename), "%s.z", argv[file]);

        // Number of used threads for pthread_join
        int threadsUsed = 0;
        size_t start = 0;
        size_t fileSizeProcessed = 0;
        for(int i = 0 ; i < numThreads ; i++) { // Thread create
            if(fileSizeProcessed >= fileSize) {
                break;
            }

            // Check if the end pointer is out of bounds
            size_t end = start + bytesPerThread > fileSize ? fileSize : start + bytesPerThread;
            // Extend the end of the segment if it splits a sequence of repeated characters
            while (end > 0 && end < fileSize && data[end] == data[end - 1]) {
                end++;
            }

            threadsUsed++;
            threadArgs[i].data = data + start;
            threadArgs[i].segSize = i != numThreads - 1 ? end - start : fileSize - start;
            fileSizeProcessed += threadArgs[i].segSize;
            threadArgs[i].outputFilename = strdup(outputFilename);
            threadArgs[i].threadID = i;
            if (pthread_create(&threads[i], NULL, compress, &threadArgs[i])) {
                fprintf(stderr, "Error creating thread\n");
                exit(1);
            }
            start = end;
        }


        fprintf(stderr, "Number of thread used: %d\n", threadsUsed);
        fprintf(stderr, "Proecessed file size: %ld\n", fileSizeProcessed);

        // Join the threads and write the output
        for (int i = 0; i < threadsUsed; ++i) {
            pthread_join(threads[i], NULL);

            FILE* outputFile = fopen(threadArgs[i].outputFilename, "ab");
            if (outputFile == NULL) {
                perror("pzip: cannot open output file");
                exit(1);
            }
            // Write the thread's output buffer to the output file
            fwrite(threadArgs[i].outputBuffer, 1, threadArgs[i].outputSize, outputFile);

            // Free the output buffer
            free(threadArgs[i].outputBuffer);

            fclose(outputFile);
        }

        munmap(data, fileSize);
        close(fd);
    }

    return 0;
}