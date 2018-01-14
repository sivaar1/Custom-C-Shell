To get the user input, I used fgetc(stdin) within a loop until the last character. I defined a MAXBUFFLENGTH for the arbitrary length when using malloc. 
I parsed the input with strtok, using space as a delimiter. After parsing the input into a char* array, I forked the process and made the child execv with the file path and other arguments (if provided). 
An error is displayed if the execv did not work (returned -1). This is all contained within a loop so it does not end after one command from the user.
