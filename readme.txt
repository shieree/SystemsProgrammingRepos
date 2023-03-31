Hello,

We put in a lot of time and effort into this, but it is unfortunately not perfect. Nonetheless, enjoy! And please have mercy.

DEBUG is by default set to 0 but if you set DEBUG to be 1 the program will print debugging messages to the terminal that show what is going on at every step.

Extensions chosen:

    Home Directory:     replacing of "~/" done in the tokenizer() layer
                        changing directory to home for "cd" with no arguments done in execute() layer

    Escape Sequences:   removal of "\\" and "\\n" sequences are done in the beginning of the tokenizer() layer
                        the rest is done as the line is translated into tokens

Limitations:

    This program does not account for when the BUFSIZE is too small to read the entire input at once.
    Thus, buffer sizes too small to read the input in one go are not guaranteed to not split a line.
    We did not have time to fix that, so please ensure that the BUFSIZE variable is larger than your input. Sorry.

Issues/Bugs:

    Piping is partially coded, and you can see what we wrote in execute(), but it is nonfunctional due to issues we could not figure out

Notes:

    .txt files created in Windows and then uploaded to the iLab Linux environment sometimes do not work when tested with this program
    For some reason during the porting, extra spaces are added after words but before the newline char \n
    For example, this file in windows:
                                        cd ..\n
                                        pwd\n
    becomes this file on linux (sometimes):
                                        cd .. \n
                                        pwd \n
    The extra spaces mess with reading the input. It is best to avoid the issue by only using .txt files created in Linux