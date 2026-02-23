#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>

#include "parser.h"
#include "writer.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    if (!main_args_validated(argc, argv))
        return 1;
    
    int status;
    struct stat st_buf;
    const char *iptr = argv[1];
    status = stat(iptr, &st_buf);
    if (status != 0) {
        fprintf(stderr, "Error: Wrong status");
        return 1;
    }
    if (S_ISDIR(st_buf.st_mode)) {
        printf("%s is a directory.\n", iptr);
        DIR *dir;
        struct dirent *entry;

        dir = opendir(iptr);
        if (dir == NULL) {
            fprintf(stderr, "Error: Could not open dir");
            return 1;
        }

        char *iptr_tmp = strdup(iptr);
        char *folder_name = basename(iptr_tmp);
        printf("foldername=%s\n", folder_name);
        while ((entry = readdir(dir)) != NULL) {
            char *name = entry->d_name;
                            printf("name=%s\n", name);

            if (name != NULL) {
                char *vm_suffix = strstr(name, ".vm");
                if (vm_suffix) {
                    char *file_root = get_file_root(name);
                    process_single_file(name, folder_name, file_root);
                }
            }
            printf("found=%s\n", name);
            
        }
    } else if (S_ISREG(st_buf.st_mode)) {
        printf("%s is a  file.\n", iptr);
        char *file_root = get_file_root(iptr);
        char output_name[ strlen(file_root) + 4];
        sprintf(output_name, "%s.asm", file_root);
        process_single_file(iptr, output_name, file_root);
    } else {
        fprintf(stderr, "Error: Neither file nor dir");
        return 1;
    }
    
    return 0;
}