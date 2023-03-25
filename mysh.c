#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#define BUFSIZE 128
#define DEBUG 1

/*  To do:
    
    fix error when files don't have a terminating \n
    account for when the buffer does not grab an entire line
    account for when the buffer splits a line, and pass the leftovers into the next read() call

*/

// This function takes in a char array and its length, parses its tokens, and converts them into a string array
char** tokenizer(char* string, int length) {
    
    // Debug printing showing the contents of string[] and the length of it
    if (DEBUG) {
        printf("string:         ");
        for (int d = 0; d < length; d++) {
            printf("%c", string[d]);
        } printf("\n");
        printf("length:         %d\n", length);
    }

    int spaceCounter = 0;
    int longestToken = 0;
    int tokenLength = 0;

    // Count how many spaces are in the string
    for (int i = 0; i < length; i++) {
        if (string[i] == ' ') {
            spaceCounter++;
        }
    }
    
    // If there are no spaces in the string, just return the entire string as a token
    if (spaceCounter == 0) {
        longestToken = length;

        // This malloc() call is equivalent to "char token[spaceCounter+1][longestToken+1]" except stored on the heap instead of the stack
        char (*token)[longestToken+1] = malloc(sizeof(char[spaceCounter+1][longestToken+1]));

        for (int i = 0; i <= length; i++) {
            if (i == length) {
                token[spaceCounter+1][i] = '\0';
                break;
            } else {
                token[spaceCounter+1][i] = string[i];
            }
        }
        if (DEBUG) {printf("----------------\n");}
        return token;
    }

    // Create an int array to store where the spaces are in the string, and reset spaceCounter
    int spaceIndexes[spaceCounter];
    spaceCounter = 0;

    // Use a single loop to simultaneously find the longest token and store where the spaces are
    for (int i = 0; i < length; i++) {
        tokenLength++;
        if (string[i] == ' ') {
            spaceIndexes[spaceCounter++] = i;
            tokenLength -= 1;
            if (tokenLength > longestToken) {
                longestToken = tokenLength;
            }
            tokenLength = 0;
        }
    }

    // Debug printing showing the length of the longest token, how many spaces there are, and where the spaces are in the string
    if (DEBUG) {
        printf("longestToken:   %d\n", longestToken);
        printf("spaceCounter:   %d\n", spaceCounter);
        if (spaceCounter == 0) {
            printf("space(s):      NULL\n");
        } else {
            printf("space(s) at:    ");
            for (int i = 0; i < spaceCounter; i++) {
                printf("%d ", spaceIndexes[i]);
            } printf("\n");
        }
        
    }

    // Create a char** token array based on how many tokens are in the string and how long the longest token is (+1 for '\0' terminator)
    // This malloc() call is equivalent to "char tokens[spaceCounter+1][longestToken+1]" except stored on the heap instead of the stack
    char (*tokens)[longestToken+1] = malloc(sizeof(char[spaceCounter+1][longestToken+1]));

    int row = 0;
    int col = 0;
    for (int i = 0; i <= length; i++) {
        if (i == length) {
            tokens[row][col] = '\0';
            break;
        } else if (string[i] == ' ') {
            tokens[row][col] = '\0';
            row++;
            i++;
            col = 0;
        }
        tokens[row][col] = string[i];
        col++;
    }

    // Debug printing that shows the contents of the tokens array
    if (DEBUG) {
        printf("----------------\n");
        printf("token(s): \n");
        for (int i = 0; i < spaceCounter+1; i++) {
            printf("                ");
            for (int j = 0; j < longestToken+1; j++) {
                if (tokens[i][j] == '\0') {
                    printf("\\0\n");
                    break;
                } else {
                    printf("%c", tokens[i][j]);
                }
            }
        }
        printf("----------------\n");
    }

    return tokens;
}

void execute(char** tokens) {

}

// This helper function allows "mySh> " to print before the standard input when in interactive mode
int myShPrinter(int interactive_mode) {
    
    if (interactive_mode == 1) {
        printf("mySh> ");
    }
    return 1;
}

void input(int input) {

    // If input == 0, then we are using standard input and we are in interactive mode
    int interactive_mode = 0;
    if (input == 0) {
        interactive_mode = 1;
    }
    
    // Create the buffer
    char* buffer = malloc(BUFSIZE*sizeof(char));
    int bytes = 0;
    int prevIndex = 0;

    // checking the conditions of the while loop simultaneous reads from the input and also prints "mySh> " if in interactive mode
    while ((myShPrinter(interactive_mode)) && (bytes = read(input, buffer, BUFSIZE)) > 0) {

        // Debug printing to show the contents of buffer[]
        if (DEBUG) {
            printf("\nDEBUG MODE ENABLED\n");
            printf("----------------\n");
            printf("buffer:         ");
            for (int i = 0; i < bytes; i++) {
                if (buffer[i] == '\n') {
                    printf("\\n\\");
                } else {
                    printf("%c", buffer[i]);
                }
            } printf("\n");
            printf("----------------\n");
        }

        // 
        for (int i = 0; i < bytes; i++) {
            if (buffer[i] == '\n') {
                
                char* line = malloc((i - prevIndex) * sizeof(char));
                int k = 0;

                for (int j = prevIndex; j < i; j++) {
                    line[k++] = buffer[j];
                }

                char** tokens = tokenizer(line, (i - prevIndex));
                
                free(line);
                free(tokens);
                prevIndex = i+1;
            }
        }
        prevIndex = 0;
        memset(buffer, 0, BUFSIZE);
    }

    free(buffer);


}

int main(int argc, char* argv[]) {

    if (argc == 1) { // Interactive mode, where there are no provided arguments

        // Print greeting
        printf("\nWelcome to myShell!\n\n");
        
        // Call input() with standard input's file descriptor passed in
        input(0);
        
    } else if (argc > 1) { // Batch mode, where the argument is a file that should be opened and interpreted

        int fd_b = open(argv[1], O_RDONLY);

        if (fd_b == -1) {
            printf("Error in mysh: unable to open specified file.\n");
            exit(EXIT_FAILURE);
        }

        // Call input() with the given file's descriptor being passed in
        input(fd_b);
    }
}