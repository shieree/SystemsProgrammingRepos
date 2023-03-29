#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
// #include <sys/wait.h>

#define BUFSIZE         128
#define NUM_COMMANDS    4
#define DEBUG           1

/*  To do:
    
    reading and prompting:
        account for when the buffer does not grab an entire line
        account for when the buffer splits a line, and pass the leftovers into the next read() call
        "!mysh> " if the last command failed

    commands to implement:

        cd:         chdir(path_to_new_directory)
        pwd:        getcwd()
        exit:       if no error, exit(0). if error, print error (perror()) to standard error, exit(1), and !mysh
        cat:    
        echo:   
        /:          first token being "/" means it is a path to an executable program
                    fork(), execv(), and wait() should be used
                    if the program cannot be executed, print error and set exit(1)
    
    if the command is not a built-in and is not a path, mysh should check the following directories for the file:
        /usr/local/sbin
        /usr/local/bin
        /usr/sbin
        /usr/bin
        /sbin
        /bin
    use stat() to check existence. if not opened set exit(1) and print error
    if file can't be executed do not look in other directories


    |:          combines sub-commands
    foo < bar:  bar considered a path and is used as standard input for foo, and is not included as an argument
    foo > bar:  same as above except bar is standard output
                file should be created if it doesn't exist or truncated if it does. use mode 0640 when creating
    use dup2() in child process to redefine fd 0 or 1 before calling execv()
    if unable to open files, report error and exit(1)

    halt batch mode if syntax error (e.g. < not followed by a word)
    if cd gets more arguments than expected, print error

    write() for the prompt?

    wildcard:   Given a path with a wildcard, determine which directory you need to check, then open the directory
                find entries whose names begin with the text before the star and end with the text after the star, and add those entries to the argument list
                If no entries match, add the original text.

    fixed:
        fix error when files don't have a terminating \n
        foo<bar is three tokens NOT separated by spaces
        "!mysh> " if the last command failed


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
    char commands[NUM_COMMANDS][10] = { "cd", "pwd", "exit", "echo"};
    //                                   0      1      2       3

    // Main execute() loop. Cross references token against the list of known commands
    // If the token matches a known command, set commandID to its matching index and carry out the specified command
    // If the token does not match a command, it is a file name and a loop searches for its following arguments and redirects
    int commandID = -1;
    for (int i = 0; i < numTokens; i++) {
        int commandID = -1;
        for (int j = 0; j < NUM_COMMANDS; j++) {
            if (strcmp(tokens[i], commands[j]) == 0) {
                commandID = j;
                break;
            }
        }

        switch (commandID) {
            
            // If the token doesn't match any known commands in the commands[] array, this runs
            // The token is treated as a path or bare name
            case -1: ; 

                // This counts the number of arguments for the given token and checks for redirects or sub-commands
                if (DEBUG) {printf("Looking for file %s:\n", tokens[i]);}
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

                // If there is a file redirect or pipe following the path/bare token, this runs
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
                char** newArgv = malloc(numArguments * sizeof(char*));
                int newArgvIndex = 0;
                int tokensNewIndex = i;
                while (newArgvIndex < numArguments) {
                    newArgv[newArgvIndex++] = tokens[tokensNewIndex++];
                }
                
                if (DEBUG) {
                    for (int k = 0; k < numArguments; k++) {
                        printf("newArgv[%d]: %s\n", k, newArgv[k]);
                    }
                }

                i = i + numArguments;
                if (redirectOrSubIndex != -1) {
                    i = i + 1;
                }

                // REMOVE BEFORE SUBMITTING. TESTING PURPOSES
                char directoryTest[100] = "C:\\Users\\Patrick\\Documents\\ComputerScience\\CS214\\test\\";
                strcat(directoryTest, tokens[i]);

                // This checks to see if the token is a bare name, and checks 6 bin directories to see if the given file is in them
                // not fully finished
                char directory1[50] = "/user/local/sbin/";  char directory2[50] = "/user/local/bin/";
                char directory3[50] = "/user/sbin/";        char directory4[50] = "/user/bin/";
                char directory5[50] = "/sbin/";             char directory6[50] = "/bin/";
                strcat(directory1, tokens[i]); strcat(directory2, tokens[i]); strcat(directory3, tokens[i]);
                strcat(directory4, tokens[i]); strcat(directory5, tokens[i]); strcat(directory6, tokens[i]);
                struct stat bufferStat;
                int bareDirectoriesCount = 7;
                char* bareDirectory[] = {directory1, directory2, directory3, directory4, directory5, directory6, directoryTest};
                int bareDirectoryFound = -1;
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
                        printf("bareDirectoryFound: %d\n", bareDirectoryFound);
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
                // Note: does not apparently work on windows since <sys/wait.h> cannot be included. Will have to be worked on in a linux environment
                
                // int pid = fork();
                // if (pid == -1) {
                //     perror("fork");
                //     exit(EXIT_FAILURE);
                // }
                // if (pid == 0) {
                //     execv(path_name, newArgv);
                //     printf("fork(): should not be seeing this!\n");
                //     exit(EXIT_FAILURE);
                // }
                // int wait_status;
                // int tpid = wait(&wait_status);
                // if (WIFEXITED(wait_status)) {
                //     printf("child exited with %d\n", WEXITSTATUS(wait_status));
                // }

                free(newArgv);

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
            
            case 3: ; // "echo"
                if (DEBUG) {printf("tokens[%d]: exiting!\n", i);}
                
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

    if (interactive_mode == 1 && errno == 0) {
        printf("mySh> ");
    } else if (interactive_mode == 1 && errno != 0) {
        printf("!mySh> ");
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

    // checking the conditions of the while loop simultaneous reads from the input and also prints "mySh> " if in interactive mode
    while ((myShPrinter(interactive_mode)) && (bytes = read(input, buffer, BUFSIZE)) > 0) {

        // Debug printing to show the contents of buffer[]
        if (DEBUG) {
            printf("\nDEBUG MODE ENABLED\n");
            printf("----------------\n");
            printf("bytes read:     %d\n", bytes);
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

        // Loop that iterates through the buffer, parses it by line, and then tokenizes and executes each line after parsing
        for (int i = 0; i <= bytes; i++) {
            if ((buffer[i] == '\n' && i != bytes) || (i == bytes && interactive_mode == 0)) {
                
                char* line = malloc((i - prevIndex) * sizeof(char));
                int k = 0;

                for (int j = prevIndex; j < i; j++) {
                    line[k++] = buffer[j];
                }

                char** tokens = tokenizer(line, (i - prevIndex));

                // printf("pointer in input(): %p\n", &tokens);
                
                execute(tokens);
                
                free(line);
                // free(tokens); // freed in execute()
                prevIndex = i+1;
            }
        }
        prevIndex = 0;
        memset(buffer, 0, BUFSIZE);
    }

    free(buffer);

}

// -----------------------------------------------------------------------------------------------------------

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