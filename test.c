#include <stdio.h>
#include <assert.h>
#include <string.h>

// Declare the function from the other file
extern void compressFile(FILE *fp);

void test_compressFile() {
    // Create a temporary file
    FILE *temp = tmpfile();
    // Write some data to the file
    fputs("AAAABBB", temp);
    // Rewind the file to the beginning
    rewind(temp);
    // Redirect stdout to a buffer
    char buffer[256];
    freopen("/dev/null", "a", stdout);
    setbuf(stdout, buffer);
    // Call the function to test
    compressFile(temp);
    // Restore stdout
    freopen("/dev/tty", "a", stdout);
    // Check the result
    assert(strcmp(buffer, "4A3B") == 0);
    // Close the temporary file
    fclose(temp);
}

int main() {
    test_compressFile();
    printf("All tests passed.\n");
    return 0;
}