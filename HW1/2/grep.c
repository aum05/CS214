#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){
    char option[] = "-i";
    size_t size = 10;
    char *str;
    str = (char*) malloc (size);
    char *original;

    if (strcmp(argv[1],option) == 0){
        original = argv[2];
        while(!feof(stdin)){
            size_t num_bytes = getline(&str, &size, stdin);
            str[strcspn(str, "\n")] = '\0';
            if (strcasestr(str,original)){
                printf("%s\n", str);
            }
        }
    }
    else {
        original = argv[1];
        while(!feof(stdin)){
            size_t num_bytes = getline(&str, &size, stdin);
            str[strcspn(str, "\n")] = '\0';
            if (strstr(str,original)){
                printf("%s\n", str);
            }
        }
    }

    return EXIT_SUCCESS;
}