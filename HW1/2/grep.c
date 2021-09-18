#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *getstring(){
    char *str = NULL, *temp = NULL;
    size_t size = 0, index = 0;
    int c = EOF;

    while (c) {
        c = getc(stdin);

        // Check whether we have reached End-of-File or New-line
        if (c == EOF || c == '\n')
            c = 0;

        // If the size of the string is less than the current index
        // We need to re-allocate memory
        if (size <= index) {
            size += 1;
            temp = realloc(str, size);
            if (!temp) {
                free(str);
                str = NULL;
                break;
            }
            str = temp;
        }

        // create the whole string by adding each character at its index
        str[index++] = c;
    }
    return str;
}

int main(int argc, char** argv){
    char option[] = "-i";
    char *original = NULL;
    char *str = NULL;
    if (strcmp(argv[1],option) == 0){
        original = argv[2];
        while(!feof(stdin)){
        str = getstring();
            if (strcasestr(str,original)){
                printf("%s\n", str);
            }
        }
    }
    else {
        original = argv[1];
        while(!feof(stdin)){
        str = getstring();
            if (strstr(str,original)){
                printf("%s\n", str);
            }
        }
    }

    return EXIT_SUCCESS;
}