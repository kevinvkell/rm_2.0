//Kevin Kell
//Project 1

#define _XOPEN_SOURCE 500
#define DIR_DEPTH 25

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <ftw.h>
#include <sys/stat.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>

void usage();

int h_flag = 0;
int current_option;
char *trash_location;
struct stat trash_directory_stat;

int main(int argc, char *argv[]) {
        while((current_option = getopt(argc, argv, "h")) != -1) {
                switch(current_option) {
                        case 'h':
                                h_flag = 1;
                                break;
                        case '?':
                                fprintf(stderr, "incorrect arguments\n");
                                exit(1);
                                break;
                }
        }

        if(h_flag != 0) {
                usage();
                return 0;
        }

        trash_location = malloc(strlen(getenv("TRASH")) +1);
        strcpy(trash_location, getenv("TRASH"));

        if(trash_location == NULL) {
                fprintf(stderr, "Please set TRASH environment variable\n");
                exit(1);
        }

        if(stat(trash_location, &trash_directory_stat) != 0) {
                perror("stat");
                exit(1);
        }

        if(nftw(trash_location, delete_directory_tree, DIR_DEPTH, FTW_CHDIR | FTW_DEPTH) != 0) {
                perror("nftw");
                exit(1);
        }

        if(mkdir(trash_location, trash_directory_stat.st_mode) != 0) {
                perror("mkdir");
                exit(1);
        }

        return 0;
}

void usage(void) {
        fprintf(stderr, "usage: dump [-h]\n");
        fprintf(stderr, "\t-h prints usage\n");
}

