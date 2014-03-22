//Kevin Kell
//Distributed Systems Project 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void print_info(char *message);
void rm_recursive();
void rm_non_recursive();

int f_flag = 0, h_flag = 0, r_flag = 0;
char *file_name = NULL;
int current_option;
char *trash_location;

int main(int argc, char *argv[]) {
	while((current_option = getopt(argc, argv, "fhr")) != -1) {
		switch(current_option) {
			case 'f':
				f_flag = 1;
				break;
			case 'h':
				h_flag = 1;
				break;
			case 'r':
				r_flag = 1;
				break;
			case '?':
				printf("incorrect arguments\n");
				exit(1);
				break;
		}
	}
	
	if(argc - optind == 1) {
		file_name = argv[optind];
	}
	else {
		printf("incorrect arguments\n");
		exit(1);
	}

	trash_location = getenv("TRASH");
	if(!trash_location) {
		printf("Please set TRASH environment variable\n");
	}
	else{
		printf("TRASH: %s \n", trash_location);
	}

	printf("f_flag: %d \nh_flag: %d \nr_flag: %d \nfilename: %s \n", f_flag, h_flag, r_flag, file_name);

	rm_non_recursive();

	return 0;
}

void print_info(char *message) {
	if(h_flag) {
		printf("%s\n", message);
	}
}

void rm_recursive() {

}

void rm_non_recursive() {
	int result;
	char destination[strlen(trash_location) + strlen(file_name) + 1];

	strcat(destination, trash_location);
	strcat(destination, file_name);

	printf("destination: %s \n", destination);
	result = rename(file_name, destination);
	if(result != 0) {
		perror("rename");
	}
	else {
		printf("Moved %s to trash\n", file_name);
	}
}

