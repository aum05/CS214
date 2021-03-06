#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Comparator function for comparing strings
static int strcompare(const void* x, const void* y){
    return strcasecmp(*(const char**)x, *(const char**)y);
}

// Comparator function for comparing integers
int intcomp(const void* x, const void* y){
    return (*(int*)x - *(int*)y);
}

// Sort function for strings
void strsort(const char* arr[], int n){
    qsort(arr, n, sizeof(const char*), strcompare);
}

// Sort function for integers
void intsort(int arr[], int n){
    qsort(arr, n, sizeof(int), intcomp);
}

char *getString(void){
    char *str = NULL;
    int c;
    size_t size = 0;

    while(EOF != (c=fgetc(stdin)) && c != '\n' ) {
        //Increase the space by 2 bytes, since one character represents 1 byte.
        str = (char*)realloc(str,  size + 2);
        str[size++] = c;
    }
    if(str)
        str[size] = '\0';
    return str;
}

int main(int argc, char** argv){
    int ind = 0;
    int count = 0;

    if(argc == 1){
        char **strings = NULL;
        char *str;
        //Store the input strings from stdin into strings array
        for(ind = 0; (str=getString()); ++ind){
            strings = (char**) realloc (strings, (ind+1)*sizeof(*strings));
            strings[ind] = str;
        }
        //Sort the strings lexicographically
        strsort((const char**)strings, ind);

        //Since each string in the strings array has been allocated memory
        //Each string needs to be free individually
        for(count = 0; count < ind; ++count){
            printf("%s\n", strings[count]);
            free(strings[count]);
        }
        free(strings);
    }
    else if (strcmp(argv[1], "-n") == 0){
        int *numbers = NULL;
        int number;
        //Store the input numbers from stdin into numbers array
        while (!feof(stdin)){
            scanf("%d\n", &number);
            numbers = (int*) realloc (numbers, (ind+1)*sizeof(*numbers));
            numbers[ind] = number;
            ++ind;
        }
        //Sort the numbers array
        intsort(numbers, ind);

        for(count = 0; count < ind; ++count){
            printf("%d\n", numbers[count]);
        }
        free(numbers);
    }
    
    return EXIT_SUCCESS;
}