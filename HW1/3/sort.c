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

// Function to dynamically allocate memory for a string read from stdin
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
    char option[] = "-n";
    
    
    return EXIT_SUCCESS;
}