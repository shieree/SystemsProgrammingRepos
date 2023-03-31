#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define BUFSIZE         8192
#define DEBUG           1

// Enjoy!

// -----------------------------------------------------------------------------------------------------------

// This function takes in a char array and its length, parses its tokens, and converts them into a string array
char** tokenizer(char* string, int length) {

    // Removing double \\ and \\n for the purpose of the escape sequence extension
    for (int i = 0; i < length; i++) {
        if (string[i] == '\\' && string[i+1] == '\\') {
            if (string[i+2] == 'n') {
                for (int j = i; j < length-3; j++) {
                    string[j] = string[j+3];
                }
                length = length - 3;
            } else {
                for (int j = i; j < length-2; j++) {
                    string[j] = string[j+2];
                }
                length = length - 2;
            }
        }
    }

    // This filters the input and adds spaces before and after special characters in case they are missing
    // For example, "foo < bar" is equivalent to "foo<bar" and this function adds in those missing spaces if applicable
    for (int i = 0; i < length; i++) {
        if ((string[i] == '|' || string[i] == '<' || string[i] == '>') && string[i-1] != '\\') {
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
        if (string[i] == ' ' && string[i-1] != '\\') {
            breakCounter++;
        }
    }
    
    // If there are no breaks in the string, just return the entire string as a token
    if (breakCounter == 0) {
        longestToken = length;

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
        if (i == length-1) {
            tokenLength++;
            if (tokenLength > longestToken) {
                longestToken = tokenLength;
            }
            tokenLength = 0;
        } else if (string[i] == ' ' && string[i-1] != '\\') {
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

    char** tokens = malloc((breakCounter+2) * sizeof(char*));
    for (int i = 0; i < breakCounter+2; i++) {
        tokens[i] = malloc((longestToken+1) * sizeof(char));
    }

    // Places the contents of string[] into tokens[][] with spaces as the row separators
    int row = 0;
    int col = 0;
    for (int i = 0; i <= length; i++) {
        if (i == length) {
            tokens[row][col] = '\0';
            break;
        } else if (string[i] == ' ' && string[i-1] != '\\') {
            tokens[row][col] = '\0';
            row++;
            i++;
            col = 0;
        }
        tokens[row][col] = string[i];
        col++;
    }

    // Home directory Extension
    // Replaces "~/" with the home directory
    for (int i = 0; i < (breakCounter+1); i++) {
        if (tokens[i][0] == '~' && tokens[i][1] == '/') {
            char* home_dir = getenv("HOME");
            int j = 0;
            while (home_dir[j] != '\0') {
                j++;
            }
            if (DEBUG) {
                printf("home directory: ");
                for (int k = 0; k < j; k++) {
                    printf("%c", home_dir[k]);
                } printf("\n");
            }
            int l = 2;
            tokens[i] = realloc(tokens[i], (longestToken+1+j));
            tokens[i][j] = '/';
            for (int k = j+1; k < longestToken+1+j; k++) {
                tokens[i][k] = tokens[i][l++];
            }
            for (int k = 0; k < j; k++) {
                tokens[i][k] = home_dir[k];
            }
            
            // Debug printing that shows the newly replaced token
            if (DEBUG) {
                printf("replaced token[%d] with: ", i);
                for (int k = 0; k < longestToken+1+j; k++) {
                    if (tokens[i][k] == '\0') {
                        printf("\\0");
                    } else {
                        printf("%c", tokens[i][k]);
                    }
                } printf("\n");
            }
        }
    }

    tokens[breakCounter+1][0] = '\0';

    // Debug printing that shows the contents of the tokens array
    // Replaced by same printing in execute()
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


    return tokens;
}

// -----------------------------------------------------------------------------------------------------------

void execute(char** tokens) {

    int numTokens = 0;
    int escapeTokensFound = 0;
    int longestToken = 0;
    int currentLength = 0;
    for (int i = 0; tokens[i][0] != '\0'; i++) {
        numTokens++;
        for (int j = 0; tokens[i][j] != '\0'; j++) {
            currentLength++;
            if (tokens[i][j] == '\\') {
                escapeTokensFound++;
            }
        }
        if (currentLength > longestToken) {
            longestToken = currentLength;
        }
    }
    if (DEBUG) {printf("numTokens: %d, escapeTokens: %d, longestToken: %d\n", numTokens, escapeTokensFound, longestToken);}

    for (int i = 0; i < numTokens; i++) {
        for (int j = 0; j < longestToken; j++) {
            if (tokens[i][j] == '\\') {
                char* newToken = malloc(longestToken * sizeof(char));
                int k = 0;
                int l = 0;
                while (k < longestToken) {
                    if (tokens[i][k] != '\\') {
                        newToken[l++] = tokens[i][k++];
                    } else {
                        k++;
                    }
                }
                free(tokens[i]);
                tokens[i] = newToken;
            }
        }
    }

    

    // Debug printing that shows the number of tokens and the input strings
    if (DEBUG) {
            int row = 0;
            printf("----------------\n");
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
    //                        0      1      2

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

        switch (commandID) {
            
            // If the token doesn't match any known commands in the commands[] array, this runs
            // The token is treated as a path or bare name
            case -1: ; 

                // This counts the number of arguments for the given token and checks for redirects or pipes
                int redirectOrSubIndex = -1;
                int numArguments = 0;
                // Checking for file redirects
                for (int k = i; k < numTokens; k++) {
                    if (strcmp(tokens[k], "<") == 0 || strcmp(tokens[k], ">") == 0) {
                        if (tokens[k+1][0] == '\0') {
                            printf("error: no argument provided for file redirection\n");
                            return;
                        } else {
                            redirectOrSubIndex = k;
                            break;
                        }
                    } else {
                        numArguments++;
                    }
                }
                int pipesPresent = 0;
                int pipesIndex = -1;
                for (int k = i; k < numTokens; k++) {
                    if (strcmp(tokens[k], "|") == 0) {
                        pipesPresent = 1;
                        pipesIndex = k;
                        break;
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

                // This checks to see if the token is a bare name, and checks 6 bin directories to see if the given file is in them
                char directory1[100] = "/usr/local/sbin/";  char directory2[100] = "/usr/local/bin/";
                char directory3[100] = "/usr/sbin/";        char directory4[100] = "/usr/bin/";
                char directory5[100] = "/sbin/";            char directory6[100] = "/bin/";
                strcat(directory1, tokens[i]); strcat(directory2, tokens[i]); strcat(directory3, tokens[i]);
                strcat(directory4, tokens[i]); strcat(directory5, tokens[i]); strcat(directory6, tokens[i]);
                struct stat bufferStat;
                int bareDirectoriesCount = 6;
                char* bareDirectory[] = {directory1, directory2, directory3, directory4, directory5, directory6};
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
                        char* stringToPrint;
                        switch (bareDirectoryFound) {
                            case 0: stringToPrint = directory1; break;
                            case 1: stringToPrint = directory2; break;
                            case 2: stringToPrint = directory3; break;
                            case 3: stringToPrint = directory4; break;
                            case 4: stringToPrint = directory5; break;
                            case 5: stringToPrint = directory6; break;
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
                    }
                }

                // fork(), create a child process, execute the child with path_name and newArgv, and wait
                // Note: the following section will throw errors when compiled on Windows since <sys/wait.h> is not available there
                struct stat bufferStat2;
                if (stat(path_name, &bufferStat2) == 0) {
                    int pid = fork();
                    if (pid == -1) {
                        perror("fork");
                        exit(EXIT_FAILURE);
                    }
                    if (pid == 0) { // Child process

                        if (DEBUG) {printf("path_name:      %s\n", path_name); printf("----------------\n");}
                        
                        // If there is a file redirect or pipe following the path/bare token, this runs
                        // This uses dup2() to change standard input/outputs for use further down
                        if (redirectOrSubIndex != -1) {
                            
                            // If redirecting standard input
                            if (strcmp(tokens[redirectOrSubIndex], "<") == 0) {
                                int fd_newInput = open(tokens[redirectOrSubIndex+1], O_RDONLY);
                                if (DEBUG) {printf("fd_newInput: %d\n", fd_newInput);}
                                dup2(fd_newInput, STDIN_FILENO);
                                // For when you are redirecting input then output in same line
                                if (strcmp(tokens[redirectOrSubIndex+2], ">") == 0) {
                                    int fd_newOutput2 = open(tokens[redirectOrSubIndex+3], O_WRONLY | O_CREAT | O_TRUNC, 0640);
                                    if (DEBUG) {printf("fd_newOutput2: %d\n", fd_newOutput2);}
                                    dup2(fd_newOutput2, STDOUT_FILENO);
                                }
                            }

                            // If redirecting standard output
                            else if (strcmp(tokens[redirectOrSubIndex], ">") == 0) {
                                int fd_newOutput = open(tokens[redirectOrSubIndex+1], O_WRONLY | O_CREAT | O_TRUNC, 0640);
                                if (DEBUG) {printf("fd_newOutput: %d\n", fd_newOutput);}
                                dup2(fd_newOutput, STDOUT_FILENO);
                                 // For when you are redirecting output then input in same line
                                if (strcmp(tokens[redirectOrSubIndex+2], "<") == 0) {
                                    int fd_newInput2 = open(tokens[redirectOrSubIndex+3], O_RDONLY);
                                    if (DEBUG) {printf("fd_newInput2: %d\n", fd_newInput2);}
                                    dup2(fd_newInput2, STDIN_FILENO);
                                }
                            }

                            // The following piping section is nonfunctional. I could not figure out how to make it work, but I did try very hard. And that's what counts.
                            // If piping occurs
                            else if (pipesPresent == 1) {
                                int fds[2];
                                fds[0] = 0; // read end of the pipe
                                fds[1] = 1; // write end of the pipe
                                if (pipe(fds) == -1) {
                                    perror("pipe");
                                    return;
                                }
                                int pid2 = fork();
                                if (pid2 == 0) { // Pipe child process
                                    dup2(fds[1], STDOUT_FILENO);
                                    close(fds[0]);
                                    close(fds[1]);
                                    // Create a new argv for the piping child process
                                    char** pipeArgv = malloc((numTokens - pipesIndex) * sizeof(char*));
                                    int indexer = pipesIndex+1;
                                    for (int l = 0; l < (numTokens-pipesIndex); l++) {
                                        pipeArgv[l] = tokens[indexer++];
                                    }
                                    char* pipe_path_name = getenv("PATH");
                                    strcat(pipe_path_name, newArgv[0]);
                                    if (DEBUG) {
                                        printf("Calling pipe execv with path: %s\n", pipe_path_name);
                                        printf("pipeArgv:\n");
                                        for (int k = 0; k < (numTokens-pipesIndex); k++) {
                                            printf("                ");
                                            printf("[%d]: %s\n", k, pipeArgv[k]);
                                        }
                                        printf("----------------\n");
                                    }
                                    execv(pipe_path_name, pipeArgv);
                                }
                                close(fds[1]);
                                int bytesPipe = 0;
                                char* bufferPipe[BUFSIZE];
                                // while ((bytes = read(fds[0], bufferPipe, BUFSIZE)) > 0) {

                                // }
                            }
                        }
                        execv(path_name, newArgv);
                        perror("execv");
                        exit(EXIT_FAILURE);
                    }
                    int wait_status;
                    int tpid = wait(&wait_status);
                    if (WEXITSTATUS(wait_status) == 1) {
                        errno = 1;
                    }
                    if (DEBUG) {
                        if (WIFEXITED(wait_status)) {
                            printf("child exited with %d\n", WEXITSTATUS(wait_status));
                        }
                    }
                } else {
                    perror("Error");
                }

                free(newArgv);

                i = i + numArguments;
                if (redirectOrSubIndex != -1) {
                    i = i + 1;
                }

                break;
            
            case 0: ; // "cd"
                
                // Home Directory Extension
                if (strcmp(tokens[i+1], "\0") == 0) {
                    char* home = getenv("HOME");
                    if (DEBUG) {printf("going to home directory: %s\n", home);}
                    int cd_home_return = chdir(home);
                    i++;
                    if (cd_home_return == -1) {
                        perror("cd");
                        errno = 1;
                    }
                    break;
                }
                if (tokens[i+2][0] != '\0') {
                    printf("cd: too many arguments provided\n");
                    return;
                }
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
    // old windows version of nxt line: while ((myShPrinter(interactive_mode)) && (bytes = read(input, buffer, BUFSIZE)) > 0) {
    while (bytes > 0) {

        // If bytes == BUFSIZE, then it can not be guaranteed that read() did not split a line, and the program prints and error and exits
        // This is bad practice since it is possible to carry over that split line to the next read() call, but I do not have time to implement that as of now :(
        if (bytes == BUFSIZE) {
            printf("Error: buffer size too small for input. Please expand the buffer size and try again.");
            exit(EXIT_FAILURE);
        }

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

                char** tokens = tokenizer(line, length);

                execute(tokens);
                
                free(line);
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