#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>

typedef struct {
    int count;
    char character;
} task_t;

typedef struct {
    task_t* tasks;
    int numTasks;
    char* output; // Decompressed data
} thread_t;

// Thread function for decompression
void* decompress(void* arg) {
    thread_t* thread = (thread_t*)arg;
    char* buffer = thread->output;
    int position = 0;
    
    for (int i = 0; i < thread->numTasks; ++i) {
        for (int j = 0; j < thread->tasks[i].count; ++j) {
            buffer[position] = thread->tasks[i].character;
            position++;
        }
    }

    return (void*)buffer; // Return the decompressed data (output buffer);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: punzip file1 [file2 ...]\n");
        exit(1);
    }

    for(int file = 1; file < argc; file++) {
        int fd = open(argv[file], O_RDONLY);
        if (fd == -1) {
            perror("Cannot open file");
            exit(1);
        }

        struct stat st;
        if (fstat(fd, &st) == -1) {
            perror("Cannot stat file");
            close(fd);
            exit(1);
        }
        size_t fileSize = st.st_size;

        char* data = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED) {
            perror("Cannot mmap file");
            close(fd);
            exit(1);
        }

        // Count the number of tasks (assume well-formed input)
        int numTasks = 0;
        size_t totalDecompressedSize = 0;
        for (size_t i = 0; i < fileSize; i += 5) {
            int count;
            memcpy(&count, data + i, sizeof(int));
            totalDecompressedSize += count;
            numTasks++;
        }

        // Create decompression tasks
        task_t* tasks = malloc(sizeof(task_t) * numTasks);
        if (!tasks) {
            perror("Failed to allocate memory for tasks");
            munmap(data, fileSize);
            close(fd);
            exit(1);
        }

        // Create an output file name with .unz extension
        char outputFilename[256];
        char* dot = strrchr(argv[file], '.');
        if (dot) *dot = '\0';  // End the string at the dot
        snprintf(outputFilename, sizeof(outputFilename), "%s_unzip", argv[file]);
        FILE* outputFile = fopen(outputFilename, "wb");
        if (!outputFile) {
            perror("Cannot open output file");
            free(tasks);
            munmap(data, fileSize);
            close(fd);
            continue; // Skip to the next file
        }

        for (size_t i = 0, taskIndex = 0; i < fileSize; i += 5, taskIndex++) {
            // Assuming the format is exactly 4 bytes for count and 1 byte for character
            int count;
            memcpy(&count, data + i, sizeof(int));
            char character = *(data + i + sizeof(int));
            tasks[taskIndex] = (task_t){count, character};
        }

        // Multithreading setup
        int numThreads = get_nprocs(); // Define the number of threads
        pthread_t threads[numThreads];
        thread_t threadArgs[numThreads];
        int tasksPerThread = numTasks / numThreads;
        if(tasksPerThread == 0) {
            tasksPerThread = 1;
        }

        // Allocate and divide tasks among threads
        size_t decompressedSizes[numThreads];
        memset(decompressedSizes, 0, sizeof(decompressedSizes));
        int tasksProcessed = 0, threadsUsed = 0;
        for (int i = 0; i < numThreads; ++i) {
            if(tasksProcessed >= numTasks) {
                break;
            }

            threadArgs[i].tasks = tasks + i * tasksPerThread;
            threadArgs[i].numTasks = (i != numThreads - 1) ? tasksPerThread : numTasks - i * tasksPerThread;
            tasksProcessed += threadArgs[i].numTasks;
            threadsUsed++;
            for (int j = 0; j < threadArgs[i].numTasks; ++j) {
                decompressedSizes[i] += threadArgs[i].tasks[j].count;
            }
            threadArgs[i].output = malloc(decompressedSizes[i] * sizeof(char)); // Allocate memory for decompressed data

            if (pthread_create(&threads[i], NULL, decompress, &threadArgs[i])) {
                perror("Error creating thread");
                exit(1);
            }
        }

        for (int i = 0; i < threadsUsed; ++i) {
            char* threadOutput;
            pthread_join(threads[i], (void**)&threadOutput);

            // Write the output of each thread to the file
            fwrite(threadOutput, 1, decompressedSizes[i], outputFile);

            free(threadOutput); // Free the output buffer allocated in each thread
        }
        fclose(outputFile); // Close the output file

        // Clean up other resources
        free(tasks);
        munmap(data, fileSize);
        close(fd);

    }

    return 0;
}
