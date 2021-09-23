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
    return str;//The value is NULL when there is no input substantial.
}

int main(int argc, char** argv){
    //char option[] = "-n";
    int index = 0;
    int count = 0;

    if(argc == 1){
        char **strings = NULL;
        char *str;
        for(index = 0; (str=getString()); ++index){
            strings = (char**) realloc (strings, (index+1)*sizeof(*strings));
            strings[index] = str;
        }
        strsort((const char**)strings, index);

        printf("\nSorted:\n");

        for(count = 0; count < index; ++count){
            printf("%s\n", strings[count]);
            free(strings[count]);
        }
        free(strings);
    }
    else if (strcmp(argv[1], "-n") == 0){
        int *numbers = NULL;
        int number;
        while (!feof(stdin))
        {
            scanf("%d\n", &number);
            numbers = (int*) realloc (numbers, (index+1)*sizeof(*numbers));
            numbers[index] = number;
            ++index;
        }

        intsort(numbers, index);

        printf("\nSorted:\n");

        for(count = 0; count < index; ++count){
            printf("%d\n", numbers[count]);
        }
        free(numbers);
    }
    

    else{
       
    }
    
    return EXIT_SUCCESS;
}