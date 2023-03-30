#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <glob.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define BUFSIZE         128
#define DEBUG           1

/*  
    ~~ To do for Patrick: ~~
    
    reading
        account for when the buffer does not grab an entire line
        account for when the buffer splits a line, and pass the leftovers into the next read() call
    implement piping
    implement changing both standard input and output at same time
    halt batch mode if syntax error (e.g. < not followed by a word)
    if cd gets more arguments than expected, print error
    write() for the prompt?
    
    extensions (choose 2):
        escape sequences
        home directory
        combining with && and ||

    ~~ To do for Shieree: ~~

    wildcard:   Given a path with a wildcard, determine which directory you need to check, then open the directory
                find entries whose names begin with the text before the star and end with the text after the star, and add those entries to the argument list
                If no entries match, add the original text.

    ~~ Things to keep in mind: ~~

    when importing Windows text files to iLab for testing purposes, the text files may not work properly. write the text files in iLab if there are issues

*/

// -----------------------------------------------------------------------------------------------------------

// This function takes in a char array and its length, parses its tokens, and converts them into a string array
char** tokenizer(char* string, int length) {

    // This filters the input and adds spaces before and after special characters in case they are missing
    // For example, "foo < bar" is equivalent to "foo<bar" and this function adds in those missing spaces if applicable
    for (int i = 0; i < length; i++) {
        if (string[i] == '|' || string[i] == '<' || string[i] == '>') {
            char operator = string[i];
            if (string[i-1] != ' ') {
                length++;
                string = realloc(string, length);
                for (int j = length-1; j > i; j--) {
                    string[j] = string[j-1];
                }
                string[i+1] = operator;
                string[i] = ' ';
                i++;
            }
            if (string[i+1] != ' ') {
                length++;
                string = realloc(string, length);
                for (int j = length-1; j > i; j--) {
                    string[j] = string[j-1];
                }
                string[i+1] = ' ';
            }
        }
    }

    
    // Debug printing showing the contents of string and the length of it
    if (DEBUG) {
        printf("string:         ");
        for (int d = 0; d < length; d++) {
            printf("%c", string[d]);
        } printf("\n");
        printf("length:         %d\n", length);
    }

    int breakCounter = 0;
    int longestToken = 0;
    int tokenLength = 0;

    // Count how many breaks are in the string
    for (int i = 0; i < length; i++) {
        if (string[i] == ' ') {
            breakCounter++;
        }
    }
    
    // If there are no breaks in the string, just return the entire string as a token
    if (breakCounter == 0) {
        longestToken = length;

        // This malloc() call is equivalent to "char token[breakCounter+2][longestToken+1]" except stored on the heap instead of the stack
        // char (*token)[longestToken+1] = malloc(sizeof(char[breakCounter+2][longestToken+1])); REMOVE

        char** token = malloc((breakCounter+2) * sizeof(char*));
        for (int i = 0; i < breakCounter+2; i++) {
            token[i] = malloc((longestToken+1) * sizeof(char));
        }

        for (int i = 0; i <= length; i++) {
            if (i == length) {
                token[breakCounter][i] = '\0';
                break;
            } else {
                token[breakCounter][i] = string[i];
            }
        }

        token[breakCounter+1][0] = '\0';

        // Debug printing that shows the contents of the tokens array
        if (DEBUG) {
            printf("----------------\n");
            printf("token(s): \n");
            for (int i = 0; i < breakCounter+2; i++) {
                printf("                ");
                for (int j = 0; j < longestToken+1; j++) {
                    if (token[i][j] == '\0') {
                        printf("\\0\n");
                        break;
                    } else {
                        printf("%c", token[i][j]);
                    }
                }
            }
            printf("----------------\n");
        }

        return token;
    }

    // Create an int array to store where the breaks are in the string, and reset breakCounter
    int breaksIndexes[breakCounter];
    breakCounter = 0;

    // Use a single loop to simultaneously find the longest token and store where the breaks are
    for (int i = 0; i < length; i++) {
        // printf("tokenLength: %d, longestToken: %d\n", tokenLength, longestToken);
        if (i == length-1) {
            tokenLength++;
            if (tokenLength > longestToken) {
                longestToken = tokenLength;
            }
            tokenLength = 0;
        } else if (string[i] == ' ') {
            breaksIndexes[breakCounter++] = i;
            if (tokenLength > longestToken) {
                longestToken = tokenLength;
            }
            tokenLength = 0;
        } else {
            tokenLength++;
        }
    }

    // Debug printing showing the length of the longest token, how many breaks there are, and where the breaks are in the string
    if (DEBUG) {
        printf("longestToken:   %d\n", longestToken);
        printf("breakCounter:   %d\n", breakCounter);
        if (breakCounter == 0) {
            printf("breaks(s):     NULL\n");
        } else {
            printf("breaks(s) at:   ");
            for (int i = 0; i < breakCounter; i++) {
                printf("%d ", breaksIndexes[i]);
            } printf("\n");
        }
        
    }

    // Create a char** token array based on how many tokens (+2) are in the string and how long the longest token is (+1)
    // This malloc() call is equivalent to "char tokens[breakCounter+2][longestToken+1]" except stored on the heap instead of the stack
    
    // char (*tokens)[longestToken+1] = malloc(sizeof(char[breakCounter+2][longestToken+1])); REMOVE

    char** tokens = malloc((breakCounter+2) * sizeof(char*));
    for (int i = 0; i < breakCounter+2; i++) {
        tokens[i] = malloc((longestToken+1) * sizeof(char));
    }

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

    tokens[breakCounter+1][0] = '\0';

    // Debug printing that shows the contents of the tokens array
    if (DEBUG) {
        printf("----------------\n");
        printf("token(s): \n");
        for (int i = 0; i < breakCounter+2; i++) {
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

    // printf("pointer in tokenizer(): %p\n", &tokens); REMOVE

    return tokens;
}

// -----------------------------------------------------------------------------------------------------------

void execute(char** tokens) {
    
    // printf("pointer in execute(): %p\n", &tokens); REMOVE
    // printf("----------------\n");

    // Count the number of tokens
    int numTokens = 0;
    while (tokens[numTokens][0] != '\0') {
        numTokens++;
    }

    // Debug printing that shows the number of tokens and the input strings
    if (DEBUG) {
            int row = 0;
            printf("numTokens:      %d\n", numTokens);
            printf("exec string(s): \n");
            while (tokens[row][0] != '\0') {
                printf("                ");
                printf("%s\n", tokens[row]);
                row++;
            }
            printf("----------------\n");
    }

    // Array of known commands used to compare against a given token
    char commands[3][10] = { "cd", "pwd", "exit"};
    //                                   0      1      2

    // Main execute() loop. Cross references token against the list of known commands
    // If the token matches a known command, set commandID to its matching index and carry out the specified command
    // If the token does not match a command, it is a file name and a loop searches for its following arguments and redirects
    for (int i = 0; i < numTokens; i++) {
        if (DEBUG) {printf("token:          %s\n", tokens[i]);}
        int commandID = -1;
        for (int j = 0; j < 3; j++) {
            if (strcmp(tokens[i], commands[j]) == 0) {
                commandID = j;
                break;
            }
        }

        // if (DEBUG) {printf("commandID: %d\n", commandID);}

        switch (commandID) {
            
            // If the token doesn't match any known commands in the commands[] array, this runs
            // The token is treated as a path or bare name
            case -1: ; 

                // This counts the number of arguments for the given token and checks for redirects or sub-commands
                int redirectOrSubIndex = -1;
                int numArguments = 0;
                for (int k = i; k < numTokens; k++) {
                    if (strcmp(tokens[k], "<") == 0 || strcmp(tokens[k], ">") == 0 || strcmp(tokens[k], "|") == 0) {
                        redirectOrSubIndex = k;
                        break;
                    } else {
                        numArguments++;
                    }
                }

                // If there is a file redirect or pipe following the path/bare token, this run
                // This uses dup2() to change standard input/outputs for use further down
                if (redirectOrSubIndex != -1) {
                    if (strcmp(tokens[redirectOrSubIndex], "<") == 0) {
                        int fd_newInput = open(tokens[redirectOrSubIndex+1], O_RDONLY);
                        dup2(fd_newInput, STDIN_FILENO);
                    } else if (strcmp(tokens[redirectOrSubIndex], ">") == 0) {
                        int fd_newOutput = open(tokens[redirectOrSubIndex+1], O_WRONLY | O_CREAT | O_TRUNC, 0640);
                        dup2(fd_newOutput, STDOUT_FILENO);
                    } else if (strcmp(tokens[redirectOrSubIndex], "|") == 0) {
                        // IMPLEMENT PIPING
                    }
                }

                // This creates a new string array and copies the token's arguments to it for use in passing to execv()
                char** newArgv = malloc((numArguments+1) * sizeof(char*));
                int newArgvIndex = 0;
                int tokensNewIndex = i;
                while (newArgvIndex < numArguments) {
                    newArgv[newArgvIndex++] = tokens[tokensNewIndex++];
                }
                newArgv[numArguments] = NULL;
                
                if (DEBUG) {
                    printf("newArgv:\n");

                    for (int k = 0; k < numArguments+1; k++) {
                        printf("                ");
                        printf("[%d]: %s\n", k, newArgv[k]);
                    }
                    printf("----------------\n");
                }
                
                glob_t gstruct;
                int wildcard = 0;
                // wildcards
                for (int a = 0; a < numTokens; a++) {
                    if (strcmp(tokens[a], "*") == 0) {
                        if (tokens[a] == GLOB_NOMATCH) {
                            printf("No matches\n");                            
                        }
                    }
                    else if (strcmp(tokens[a], '*') == 1) {
                        printf("Found %zu filename matches\n", gstruct.gl_pathv);
                        tokens[a] = gstruct.gl_pathv;

                        printf("%s\n", tokens[a]);
                    }
                }
                // REMOVE BEFORE SUBMITTING. TESTING PURPOSES
                char directoryTest[100] = "/common/home/pfb34/214/myShell/";
                strcat(directoryTest, tokens[i]);

                // This checks to see if the token is a bare name, and checks 6 bin directories to see if the given file is in them
                // not fully finished
                char directory1[100] = "/usr/local/sbin/";  char directory2[100] = "/usr/local/bin/";
                char directory3[100] = "/usr/sbin/";        char directory4[100] = "/usr/bin/";
                char directory5[100] = "/sbin/";            char directory6[100] = "/bin/";
                strcat(directory1, tokens[i]); strcat(directory2, tokens[i]); strcat(directory3, tokens[i]);
                strcat(directory4, tokens[i]); strcat(directory5, tokens[i]); strcat(directory6, tokens[i]);
                struct stat bufferStat;
                int bareDirectoriesCount = 7;
                char* bareDirectory[] = {directory1, directory2, directory3, directory4, directory5, directory6, directoryTest};
                int bareDirectoryFound = -1;
                
                // for (int o = 0; o < bareDirectoriesCount; o++) {
                //     printf("bareDirectory[%d]: %s\n", o, bareDirectory[o]);
                // }

                for (int k = 0; k < bareDirectoriesCount; k++) {
                    if (stat(bareDirectory[k], &bufferStat) == 0) {
                        bareDirectoryFound = k;
                        break;
                    }
                }
                errno = 0;
                if (DEBUG) {
                    if (bareDirectoryFound == -1) {
                        printf("file not found in bareDirectory\n");
                    } else {
                        char* stringToPrint;
                        switch (bareDirectoryFound) {
                            case 0: stringToPrint = directory1; break;
                            case 1: stringToPrint = directory2; break;
                            case 2: stringToPrint = directory3; break;
                            case 3: stringToPrint = directory4; break;
                            case 4: stringToPrint = directory5; break;
                            case 5: stringToPrint = directory6; break;
                            case 6: stringToPrint = directoryTest; break; // REMOVE BEFORE SUBMISSION
                        }
                        printf("file found in:  %s\n", stringToPrint);
                    }
                }

                // If the file was found in a bare directory, set path_name to be that directory/token[i]
                char* path_name = tokens[i];
                if (bareDirectoryFound != -1) {
                    switch (bareDirectoryFound) {
                        case 0: path_name = directory1; break;
                        case 1: path_name = directory2; break;
                        case 2: path_name = directory3; break;
                        case 3: path_name = directory4; break;
                        case 4: path_name = directory5; break;
                        case 5: path_name = directory6; break;
                        case 6: path_name = directoryTest; break; // REMOVE BEFORE SUBMISSION
                    }
                }

                // fork(), create a child process, execute the child with path_name and newArgv, and wait
                // Note: the following section will throw errors when compiled on Windows since <sys/wait.h> is not available
                struct stat bufferStat2;
                if (stat(path_name, &bufferStat2) == 0) {
                    int pid = fork();
                    if (pid == -1) {
                        perror("fork");
                        exit(EXIT_FAILURE);
                    }
                    if (pid == 0) {
                        if (DEBUG) {printf("path_name:      %s\n", path_name); printf("----------------\n");}
                        execv(path_name, newArgv);
                        perror("execv");
                        exit(EXIT_FAILURE);
                    }
                    int wait_status;
                    int tpid = wait(&wait_status);
                    if (DEBUG) {
                        if (WIFEXITED(wait_status)) {
                            printf("child exited with %d\n", WEXITSTATUS(wait_status));
                        }
                    }
                } else {
                    perror("error");
                }



                free(newArgv);

                i = i + numArguments;
                if (redirectOrSubIndex != -1) {
                    i = i + 1;
                }

                break;
            
            case 0: ; // "cd"
                char* cd_path = tokens[i+1];
                int cd_return = chdir(cd_path);
                if (DEBUG) {printf("tokens[%d]: cd %s, return: %d\n", i, cd_path, cd_return);}
                i++;
                if (cd_return == -1) {
                    perror("cd");
                    errno = 1;
                }
                break;
            
            case 1: ; // "pwd"
                char current_directory[100];
                printf("%s\n", getcwd(current_directory, 100));
                break;
            
            case 2: ; // "exit"
                printf("mySh: exiting\n");
                exit(0);
                break;
        }
    }

    for (int i = 0; i < numTokens+1; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

// This helper function allows "mySh> " to print before the standard input when in interactive mode
int myShPrinter(int interactive_mode) {
    // if (DEBUG) {printf("errno: %d, interactive_mode: %d\n", errno, interactive_mode);}
    if (interactive_mode == 1 && errno == 0) {
        printf("mySh> ");
        fflush(stdout);
    } else if (interactive_mode == 1 && errno != 0) {
        printf("!mySh> ");
        fflush(stdout);
        errno = 0;
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
    myShPrinter(interactive_mode);
    bytes = read(input, buffer, BUFSIZE);

    // checking the conditions of the while loop simultaneous reads from the input and also prints "mySh> " if in interactive mode
    // while ((myShPrinter(interactive_mode)) && (bytes = read(input, buffer, BUFSIZE)) > 0) {
    while (bytes > 0) {

        // Debug printing to show the contents of buffer[]
        if (DEBUG) {
            printf("\nDEBUG MODE ENABLED\n");
            printf("----------------\n");
            printf("bytes read:     %d\n", bytes);
            printf("buffer:\n");
            // printf("                ");
            for (int i = 0; i < bytes; i++) {
                if (buffer[i] == '\n') {
                    printf("buffer[%d]: \\n\\ \n", i);
                } else {
                    printf("buffer[%d]: %c\n", i, buffer[i]);
                }
            } printf("\n");
            printf("\n----------------\n");
        }

        // Loop that iterates through the buffer, parses it by line, and then tokenizes and executes each line after parsing
        for (int i = 0; i < bytes; i++) {
            
            // windows version of this next line: if ((buffer[i] == '\n' && i != bytes) || (i == bytes && interactive_mode == 0)) {
            // linux version does not need the extra stuff because all linux text files end with '\n' but windows .txts don't (I think?)
            if (buffer[i] == '\n') {
                
                if (DEBUG) {printf("i: %d, prevIndex: %d\n", i, prevIndex);}
                int length = (i - prevIndex);
                // if (interactive_mode == 0) {
                //     length--;
                // }

                char* line = malloc(length * sizeof(char));
                int k = 0;

                for (int j = prevIndex; j < i; j++) {
                    line[k++] = buffer[j];
                }

                // if(DEBUG) {printf("passing line of length %d into tokenizer()\n", length);}
                char** tokens = tokenizer(line, length);

                // printf("pointer in input(): %p\n", &tokens);
                
                execute(tokens);
                
                free(line);
                // free(tokens); // freed in execute()
                prevIndex = i+1;
            }
        }
        
        myShPrinter(interactive_mode);
        prevIndex = 0;
        memset(buffer, 0, BUFSIZE);
        bytes = read(input, buffer, BUFSIZE);
    }

    free(buffer);

}

// -----------------------------------------------------------------------------------------------------------

// Main function filters interactive mode and batch mode, depending on the number of provided arguments. All arguments past argv[1] are discarded in batch mode
int main(int argc, char* argv[]) {

    if (argc == 1) { // Interactive mode, where there are no provided arguments

        // Print greeting
        printf("\nWelcome to myShell!\n\n");
        
        // Call input() with standard input's file descriptor passed in
        input(0);
        
    } else if (argc > 1) { // Batch mode, where the argument is a file that should be opened and interpreted

        int fd_b = open(argv[1], O_RDONLY);

        if (fd_b == -1) {
            perror("mySh");
            exit(EXIT_FAILURE);
        }

        // Call input() with the given file's descriptor being passed in
        input(fd_b);
    }
}