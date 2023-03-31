#include <stdio.h>

#define MAX_LINE_LENGTH 256 // Define the maximum length of a line

int main() {
    FILE* file = fopen("addresses.txt", "r"); // Open the file for reading
    char line[MAX_LINE_LENGTH]; // Define a buffer for the line
    
    if (file == NULL) { // Check if the file was successfully opened
        printf("Failed to open the file.\n");
        return 1;
    }
    
    
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) { // Read each line from the file
        
        
    }
    
    fclose(file); // Close the file
    
    return 0;
}

