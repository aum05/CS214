#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
int main(int argc, char **argv)
{
    struct passwd *pw;
    struct group *gp;
    DIR *mydir;
    char *c;
    int i;
    struct dirent *myfile;
    struct stat fileStat;
    mydir=opendir(".");
    stat(".",&fileStat); 
    while((myfile=readdir(mydir))!=NULL)
    {
        if (myfile->d_name[0] == '.')
			continue;
        
        stat(myfile->d_name,&fileStat);
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
        pw=getpwuid(fileStat.st_uid);
        printf("%s ",pw->pw_name);
        gp=getgrgid(fileStat.st_gid);
        printf("%s ",gp->gr_name);
        printf("%4lld ",fileStat.st_size);
        c=ctime(&fileStat.st_mtime);
        for(i=4;i<=15;i++)
        printf("%c",c[i]);
        printf(" ");
        printf("%s\n",myfile->d_name);
    }
    closedir(mydir);  
    return 0;
}