#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    int str_count = 0;


    char **strings = NULL;
    char *str;

    //Store the input strings from stdin into strings array
    for(ind = 0; (str=getString()); ++ind){
        strings = (char**) realloc (strings, (ind+1)*sizeof(*strings));
        strings[ind] = str;
    }

    char **uniq = NULL;
    int index = 0;
    int i;
    int count = 1;
    char *num;
    //Find unique strings
    for(i = 0; i < ind; i++){
        uniq = (char**) realloc (uniq, (index+1)*sizeof(*uniq));
        if (asprintf(&num, "%d", count) == -1) {
            perror("asprintf");
        }
        if(index == 0){
            uniq[index] = strcat(num, strcat(" ", strings[index]));
            ++index;
        }
        else if (strcmp(strings[i],strings[i-1]) == 0){
            ++count;
            asprintf(&num, "%d", count);
            uniq[index] = strcat(num, strcat(" ", strings[i]));
        }
        else{
            count = 1;
            asprintf(&num, "%d", count);
            uniq[index] = strcat(num, strcat(" ", strings[i]));
            ++index;
        }
    }

    free(num);

    //Since each string in the strings array has been allocated memory
    //Each string needs to be free individually
    for(str_count = 0; str_count < index; ++str_count){
        printf("%s\n", uniq[str_count]);
        free(uniq[str_count]);
    }
    free(uniq);
    for(str_count = 0; str_count < ind; ++str_count){
        free(strings[str_count]);
    }
    free(strings);
    
    return EXIT_SUCCESS;
}