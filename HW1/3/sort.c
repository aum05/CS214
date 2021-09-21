#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Comparator function for comparing strings
static int strcompare(const void* x, const void* y){
    return strcmp(*(const char**)x, *(const char**)y);
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
void intsort(int* arr[], int n){
    qsort(arr, n, sizeof(int), intcomp);
}


void DeleteStrings(char ***strings, int size) {
    int i;
    for (i = 0; i < size; i++)
        free((*strings)[i]);
    free(*strings);
    *strings = NULL;
}

/* Print error message and exit. */
void die(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

char **StoreStrings(int *size) {
    int allocated = INITIAL_STRING_ARRAY_SIZE;
    char **strings, **temp;

    *size = 0;
    strings = malloc(allocated * sizeof(char*));
    if (strings == NULL)
        die("StoreStrings 1");

    printf("Enter strings. Enter an empty string to quit.\n");

    while (1) {
        char *str = GetString();
        if (str == NULL) {
            DeleteStrings(&strings, *size);
            die("StoreStrings 2");
        }

        if (str[0] == '\0') {
            free(str);
            break;
        }

        if (*size == allocated) {
            temp = realloc(strings, (allocated *= 2) * sizeof(char*));
            if (temp == NULL) {
                DeleteStrings(&strings, *size);
                die("StoreStrings 3");
            }
            strings = temp;
        }

        strings[(*size)++] = str;
    }

    /* Fit to size. */
    temp = realloc(strings, *size * sizeof(char*));
    if (temp == NULL) {
        DeleteStrings(&strings, *size);
        die("StoreStrings 4");
    }

    return temp;
}

int main(int argc, char** argv){
    char option[] = "-n";
    
    
    return EXIT_SUCCESS;
}