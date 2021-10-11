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

// Comparator function for comparing strings
static int strcompare(const void* x, const void* y){
    return strcasecmp(*(const char**)x, *(const char**)y);
}

// Sort function for strings
void strsort(const char* arr[], int n){
    qsort(arr, n, sizeof(const char*), strcompare);
}

int main(int argc, char** argv)
{
    DIR *dr;
    char *mod_time;
    int i;
    struct dirent *fd;
    struct stat fileStat;
    struct passwd *user_id;
    struct group *grp;

    dr=opendir(".");
    stat(".",&fileStat);

    char option[] = "-l";
    
    // If the command is only ls
    if (argc == 1){
        while((fd=readdir(dr))!=NULL) {
            if (fd->d_name[0] == '.')
                continue;

            printf("%s\n",fd->d_name);
        }
    }
    // If the command is ls -l
    else if (strcmp(argv[1],option) == 0){
        while((fd=readdir(dr))!=NULL) {
            if (fd->d_name[0] == '.')
                continue;
            // Print the 10-character permissions string
            stat(fd->d_name,&fileStat);
            printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-"); 
            printf(" ");

            // Print the file owner's user name
            user_id=getpwuid(fileStat.st_uid);
            printf("%s ",user_id->pw_name);
            
            // Print the group name
            grp=getgrgid(fileStat.st_gid);
            printf("%s ",grp->gr_name);

            // Print the file size
            printf("%4lld ",fileStat.st_size);
            
            // Print the last modification time
            mod_time = ctime(&fileStat.st_mtime);
            for(i=4;i<=15;i++)
            printf("%c",mod_time[i]);
            printf(" ");
            
            // Print the file name
            printf("%s\n",fd->d_name);
        }
    }
    closedir(dr);  
    return 0;
}