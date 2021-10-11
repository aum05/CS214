#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

int sort_func(const struct dirent ** a, const struct dirent **b) {
  return(strcasecmp((*(const struct dirent **)a)->d_name,
                    (*(const struct dirent **)b)->d_name));
}

int filter(const struct dirent *name){
  return 1;
}

int main(int argc, char** argv){
    char *mod_time;
    int i;
    int j = 0;
    struct dirent **nameList;
    int n;
    struct stat stats;
    struct passwd *user_id;
    struct group *grp;

    n = scandir(".", &nameList, filter, sort_func);

    stat(".",&stats);

    char option[] = "-l";
    
    // If the command is only ls
    if (argc == 1){
        while(j<n) {
            if (nameList[j]->d_name[0] == '.'){
                free(nameList[j]);
                ++j;
                continue;
            }

            printf("%s\n", nameList[j]->d_name);
            free(nameList[j]);
            ++j;
        }
    }
    // If the command is ls -l
    else if (strcmp(argv[1],option) == 0){
        while(j<n) {
            if (nameList[j]->d_name[0] == '.'){
                free(nameList[j]);
                ++j;
                continue;
            }
            
            // Print the 10-character permissions string
            stat(nameList[j]->d_name,&stats);
            printf( (S_ISDIR(stats.st_mode)) ? "d" : "-");
            printf( (stats.st_mode & S_IRUSR) ? "r" : "-");
            printf( (stats.st_mode & S_IWUSR) ? "w" : "-");
            printf( (stats.st_mode & S_IXUSR) ? "x" : "-");
            printf( (stats.st_mode & S_IRGRP) ? "r" : "-");
            printf( (stats.st_mode & S_IWGRP) ? "w" : "-");
            printf( (stats.st_mode & S_IXGRP) ? "x" : "-");
            printf( (stats.st_mode & S_IROTH) ? "r" : "-");
            printf( (stats.st_mode & S_IWOTH) ? "w" : "-");
            printf( (stats.st_mode & S_IXOTH) ? "x" : "-"); 
            printf(" ");

            // Print the file owner's user name
            user_id=getpwuid(stats.st_uid);
            printf("%s ",user_id->pw_name);
            
            // Print the group name
            grp=getgrgid(stats.st_gid);
            printf("%s ",grp->gr_name);

            // Print the file size
            printf("%4ld ",stats.st_size);
            
            // Print the last modification time
            mod_time = ctime(&stats.st_mtime);
            for(i=4;i<=15;i++){
                printf("%c",mod_time[i]);
            }
            printf(" ");
            
            // Print the file name
            printf("%s\n",nameList[j]->d_name);

            free(nameList[j]);
            ++j;
        }
    }
    free(nameList);
    return EXIT_SUCCESS;
}