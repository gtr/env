/*
 * env.c
 *
 * usage:
 * ./env [-i] key1=name1 key2=name2 [commands]
 * 
 *  -i:
 * Construct a new environment from scratch; normally the
 * environment is inherited from the parent process, except as
 * modified by other options.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

extern char** environ;

/*
 * displayEnvironment is adapted from The Linux Programming Interface pg. 127
 */
void displayEnviornment() {
    for (char **ep = environ; *ep != NULL; ep++) {
        puts(*ep);
    }
}

int findLengthOfArray(char** arr) {
    int count = 0;
    for (; *arr != NULL; arr++) {
        count++;
    }
    return count;
}

void runCommand(char** arguments) {
    if (execvp(arguments[0], arguments) < 0) {
        perror("runCommand() failed");
    }
}

/*
 * haveTheSameKey determines if two namne/value pair (as c strings) have the 
 * same key
 *
 * returns: true (1) if the two pairs have the same key or false (0) otherwise
 */
int haveTheSameKey(char* pair1, char* pair2) {
    while (*pair1 != '=') {
        if (*pair1 != *pair2) {
            return FALSE;
        }
        pair1++;
        pair2++;
    }
    return TRUE;
}

/*
 * createStringArray allocates an array of char pointers
 */
char** createStringArray(int length) {
    char **new_env = (char **)malloc((length + 1) * sizeof(char*));
    for (int i = 0; i < length; i++) {
        // Allocate a char pointer at each spot.
        new_env[i] = (char *)malloc(sizeof(char*));
    }
    return new_env;
}

/*
 * findIndex finds the index of a name/value pair in a string_array if its 
 * key already exists. If it doesn't, it returns the next vacant index.
 */
int findIndex(char** string_array, char* pair) {
    int index = 0;
    int length = findLengthOfArray(string_array);
    for (; string_array[index] != NULL; index++) {
        if (strcmp(string_array[index], "") == 0) {
            return index;
        }
        if (string_array[index] == pair) {
            return length;
        }
        if (haveTheSameKey(pair, string_array[index]) == TRUE) {
            return index;
        }
    }
    return index;
}
 
/*
 * isUnique determines if the key in a key/value (name/value) pair already 
 * exists in a string_array.
 */
int isUnique(char** string_array, char* pair) {
    int length = findLengthOfArray(string_array);
    int index = findIndex(string_array, pair);
    return length == index;
}

/*
 * setNameValuePair updates a name/value pair to a string_array if its key (name)
 * already exist and appends it to the end if it doesn't exist.
 */
void setNameValuePair(char** string_array, char* pair) {
    int index = findIndex(string_array, pair);
    string_array[index] = pair;
}

/*
 * parseNameValuePairs parses argv to find the name/value pairs and stores them
 * into a string array.
 * 
 * argc:    the number of arguments
 * argv:    the argument array
 * start:   the index to start searching for name/value pairs
 *
 * returns: the newly allocated string array
 */
char** parseNameValuePairs(int argc, char *argv[], int start) {
    // Find the length of the name/value pairs
    int length = 0;
    for (int i = start; i < argc; i++) {
        if (strchr(argv[i], '=') != NULL) {
            length += isUnique(argv, argv[i]);
        }
    }
    // Allocate a string array of the given length.
    char **pairs= createStringArray(length);
    
    // Determine which of the name/value pairs should be replaced or should be
    // appended.
    for (int i = start; i < argc; i++) {
        if (strchr(argv[i], '=') != NULL) {
            setNameValuePair(pairs, argv[i]);
        }
    }

    pairs[length] = NULL;
    return pairs;
}

/*
 * findCommandIndex finds the index at which a command should start in argv.
 */
int findCommandIndex(int argc, char *argv[], int start) {
    int i = start;
    while (i < argc && (strchr(argv[i], '=') != NULL)) {
        i++;
    }
    return i;
}

/*
 * parseCommand parses the command and its arguments after the name/value
 * pairs in the original argument array.
 *
 * command_index:   the index at which the command starts in the original array
 * argc:            the number of original arguments
 * argv:            the original argument array
 *
 * returns:         a string array containing the command and its arguments
 */
char** parseCommand(int command_index, int argc, char *argv[]) {
    int length = argc - command_index;
    char** commands = createStringArray(length);
    for(int i = 0; i < length; i++, command_index++) {
        commands[i] = argv[command_index];
    }
    commands[length] = NULL;
    
    return commands;
}

/*
 * mergeEnvironments merges the old environment and the new environment into a
 * merged environment where the existing name/value pairs from the new 
 * environment are merged in the old environment and unique name/value pairs
 * from the new environment are added to the new environment. 
 * 
 * old_env: the existing environment (`extern environ`)
 * new_env: the new environment provided by the user
 * 
 * returns: the resulting environment with merged name/value pairs
 */
char** mergeEnvironments(char** old_env, char** new_env) {
    // Find the size of the size of the old and new environments if they were 
    // merged (existing keys' values are replaced and new keys are appended).
    int old_env_length = findLengthOfArray(old_env);
    int new_env_length = 0;
    for (char** ptr = new_env; *ptr != NULL; ptr++) {
        new_env_length += isUnique(old_env, *ptr);
    }
    
    // Allocate a new char* array that is the combined unique size of the 
    // old and new environment.
    char** merged_env = createStringArray(new_env_length+old_env_length+1);
    
    // Copy over the old name/value pairs from old_env.
    int ptr = 0;
    for (int i = 0; old_env[i] != NULL; i++, ptr++) {
        merged_env[ptr] = old_env[i];
    }

    // Determine which of the new name/value pairs should be replaced or should
    // be appended.
    for (int i = 0; new_env[i] != NULL; i++) {
        setNameValuePair(merged_env, new_env[i]);
    }

    // Set the last index of the merged environment to NULL.
    merged_env[new_env_length+old_env_length] = NULL;
    return merged_env;

}

/*
 * setEnvironment creates a new environment either by replacing the already
 * existing environment with the name/value pairs provided or by appending them
 * to the already existing environment.
 * 
 * new_env:     the new environment provided by the user
 * ignore_env:  if the current environment should be ignored (-i flag)
 */
void setEnvironment(char** new_env, int ignore_env) {
    if (ignore_env == 1) {
        environ = new_env;
    } else {
        // Merge the the current environent and the environment provided by 
        // the user.
        char** merged_env = mergeEnvironments(environ, new_env);

        // Make this merged environment the new environment.
        environ = merged_env;
    }
}

int main(int argc, char *argv[]) {
    int start = 1;          // the start of the arg list parsing
    int ignore_env = 0;     // whether we use the `-i` option
    
    // If issued without arguments, it just displays default environment.
    if (argc == 1) {
        displayEnviornment();
        exit(0);
    }
    if (strcmp(argv[1], "-i") == 0) {
        start++;
        ignore_env = 1;
    }
    
    // Parse the name/value pairs provided by the user and create an 
    // environment for them.
    char** new_env = parseNameValuePairs(argc, argv, start);
    setEnvironment(new_env, ignore_env);

    // Parse commands (if applicable).
    int command_index = findCommandIndex(argc, argv, start);
    if (command_index < argc) {
        // If there are any commands, run them.
        char** commands = parseCommand(command_index, argc, argv);
        runCommand(commands);
        free(commands);
    } else {
        // Otherwise, just display the new/merged environment.
        displayEnviornment();
    }

}
