#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Node {
    char *str;
    int count;
    struct Node* next;
};

struct Node* add(struct Node* head, char *str, int count){
    struct Node* current = head;
    struct Node* newNode = (struct Node*) malloc(sizeof(struct Node));

    newNode->str = malloc(strlen(str)+1);
    strcpy(newNode->str, str);
    newNode->count = count;
    newNode->next = NULL;

    //If the linked list is empty
    //That means the head node will be the first node
    if (head == NULL){
        head = newNode;
        return head;
    }
    
    while (current->next != NULL){
        current = current->next;
    }

    //Once we have reached the last node of the current linked list
    //Append the head node to the last node
    //Making the head node the last node
    current->next = newNode;
    return newNode;
}

void printUniqStrs(struct Node *head){
  while (head != NULL)
  {
    printf("%d %s\n", head->count, head->str);
    head = head->next;
  }
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
    struct Node* head = NULL;
    struct Node* curr = NULL;
    int ind = 0;
    int count = 1;
    int str_count = 0;
    char **strings = NULL;
    char *str;

    //Store the input strings from stdin into strings array
    for(ind = 0; (str=getString()); ++ind){
        strings = (char**) realloc (strings, (ind+1)*sizeof(*strings));
        strings[ind] = str;
    }

    for(int i = 0; i<ind; ++i){
        if(i == 0){
            head = add(head,strings[i],count);
            curr = head;
            continue;
        }
        if(strcmp(strings[i], strings[i-1]) == 0){
            ++count;
            curr->count = count;
        }
        else{
            count = 1;
            curr = add(curr,strings[i],count);
        }
    }

    printUniqStrs(head);

    //Since each string in the strings array has been allocated memory
    //Each string needs to be free individually
    for(str_count = 0; str_count < ind; ++str_count){
        free(strings[str_count]);
    }
    free(strings);

    struct Node *p1 = head, *p2;
    while(p1){
        p2 = p1;
        p1 = p1->next;
        free(p2->str);
        free(p2);
    }
    
    return EXIT_SUCCESS;
}