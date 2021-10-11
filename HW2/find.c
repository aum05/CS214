#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <locale.h>

#define PATH_MAX 1000

static int find(const char *orgDir, const char *str)
{
	char path[PATH_MAX + 2];
	char *p = path;
	char *end = &path[PATH_MAX];

	/* Copy directory name to path */
	const char *src = orgDir;
	while (p < end && *src != '\0') {
		*p++ = *src++;
	}
	*p = '\0';

	DIR *dir = opendir(orgDir);
	if (!dir) {
		fprintf(stderr,
			"Cannot open %s (%s)\n", orgDir, strerror(errno));
		return 0;
	}

	/* Print all files and directories within the directory */
	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		char *q = p;
		char c;

		/* Get final character of directory name */
		if (path < q)
			c = q[-1];
		else
			c = ':';

		/* Append directory separator if not already there */
		if (c != ':' && c != '/' && c != '\\')
			*q++ = '/';

		/* Append file name */
		src = ent->d_name;
		while (q < end && *src != '\0') {
			*q++ = *src++;
		}
		*q = '\0';

		/* Decide what to do with the directory entry */
		switch (ent->d_type) {
            case DT_REG:
                /* Output file name with directory */
                if (strstr(ent->d_name,str)){
                    printf("%s\n", path);    			
                }
                break;
            case DT_DIR:
                /* Scan sub-directory recursively */
                if (strcmp(ent->d_name, ".") != 0
                    &&  strcmp(ent->d_name, "..") != 0) {
                    find(path,str);
                }
                break;

            default:
                /* Ignore device entries */
                /*NOP*/;
		}

	}

	closedir(dir);
	return 1;
}

int main(int argc, char **argv) {
    if (argc==1){
        perror("ENTER PATTERN");
        return EXIT_FAILURE;
    }

	find(".", argv[1]);

	return EXIT_SUCCESS;
}
