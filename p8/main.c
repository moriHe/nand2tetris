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
        while ((entry = readdir(dir)) != NULL) {
            char *name = entry->d_name;

            if (name != NULL) {
                char *vm_suffix = strstr(name, ".vm");
                if (vm_suffix) {
                    char *file_root = get_file_root(name);
                    char *iptr_single_file;
                    asprintf(&iptr_single_file, "%s/%s", iptr, name);
                    char *output_name;
                    asprintf(&output_name, "%s.asm", folder_name);
                    process_single_file(iptr_single_file, output_name, file_root, true);
                }
            }            
        }
    } else if (S_ISREG(st_buf.st_mode)) {
        printf("%s is a  file.\n", iptr);
        char *file_root = get_file_root(iptr);
        char output_name[ strlen(file_root) + 4];
        sprintf(output_name, "%s.asm", file_root);
        process_single_file(iptr, output_name, file_root, false);
    } else {
        fprintf(stderr, "Error: Neither file nor dir");
        return 1;
    }
    
    return 0;
}