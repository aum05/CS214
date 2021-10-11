#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define PATH_MAX 1000

void find(const char *orgDir, const char *str)
{
	char path[PATH_MAX + 2];
	char *i = path;
	char *lastChar = &path[PATH_MAX];

    // Add the original directory name to the path
	const char *name = orgDir;
	while (i < lastChar && *name != '\0') {
		*i++ = *name++;
	}
	*i = '\0';

	DIR *directory = opendir(orgDir);
	if (!directory) {
		printf("Unable to open directory: %s\n", orgDir);
		return;
	}

	// Traverse through the curent and subdirectories to find all filenames matching the pattern
	struct dirent *entry;
	while ((entry = readdir(directory)) != NULL) {
		char *j = i;
		char c;

		// Obtain the end character of the path
		if (path < j)
			c = j[-1];
		else
			c = ':';

		// Append the separator to end of the path if applicable
		if (c != '/' && c != ':' &&  c != '\\')
			*j++ = '/';

		// Get the filename and append it at the end of the path
		name = entry->d_name;
		while (j < lastChar && *name != '\0') {
			*j++ = *name++;
		}
		*j = '\0';

		// Perform apt action depending on whether the entry 
        // is another subdirectory or a file that matches the pattern
		switch (entry->d_type) {
            case DT_REG:
                // If it's a filename that matches the pattern, print the whole path of the file
                if (strstr(entry->d_name,str)){
                    printf("%s\n", path);    			
                }
                break;
            case DT_DIR:
                // If it's a subdirectory, recursively perform the find function
                if (strcmp(entry->d_name, ".") != 0 &&  strcmp(entry->d_name, "..") != 0) {
                    find(path,str);
                }
                break;

            default:
                // No action to perform in this case
                ;
		}

	}

	closedir(directory);
	return;
}

int main(int argc, char **argv) {
    if (argc==1){
        perror("ENTER PATTERN");
        return EXIT_FAILURE;
    }

	find(".", argv[1]);

	return EXIT_SUCCESS;
}
