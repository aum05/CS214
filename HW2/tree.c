#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int sort_func(const struct dirent ** a, const struct dirent **b) {
  return(strcasecmp((*(const struct dirent **)a)->d_name,
                    (*(const struct dirent **)b)->d_name));
}

int filter(const struct dirent *name){
  return 1;
}

void printdir(char *dir, int indent){
    int i = 0;
    struct dirent **nameList;
    int n;
    struct stat stats;

    n = scandir(dir, &nameList, filter, sort_func);

    chdir(dir);
    while(i<n) {
        lstat(nameList[i]->d_name, &stats);
        if(S_ISDIR(stats.st_mode)) {
            if(strcmp(".",nameList[i]->d_name) == 0 || strcmp("..",nameList[i]->d_name) == 0){
                free(nameList[i]);
                ++i;
                continue;
            }
            
            printf("%*s- %s\n",indent,"",nameList[i]->d_name);
            printdir(nameList[i]->d_name,indent+2);
        }

        else{ 
            printf("%*s- %s\n",indent,"",nameList[i]->d_name);
        }
        free(nameList[i]);
        ++i;
    }
    chdir("..");
    free(nameList);
}

int main(int argc, char** argv){
    printf(".\n");
    printdir(".",0);
    return EXIT_SUCCESS;
}