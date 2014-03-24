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
char *name_trash_file();
char *try_file_name(char* name_to_try, int number_to_try);

int f_flag = 0, h_flag = 0, r_flag = 0;
char *file_name;
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
	if(trash_location == NULL) {
		printf("Please set TRASH environment variable\n");
		exit(1);
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
	char *destination;

	destination = name_trash_file();

	printf("destination: %s \n", destination);
	result = rename(file_name, destination);
	free(destination);
	if(result != 0) {
		perror("rename");
	}
	else {
		printf("Moved %s to trash\n", file_name);
	}
}

char *name_trash_file() {
	char *trash_file_name;
	trash_file_name = calloc((strlen(trash_location) + strlen(file_name) + 1), sizeof(char));
	if(trash_file_name == NULL) {
		printf("could not allocate memory\n");
		exit(1);
	}

	strcat(trash_file_name, trash_location);
	strcat(trash_file_name, file_name);
	if(access(trash_file_name, F_OK) == 0) {
		trash_file_name = try_file_name(trash_file_name, 1);
	}

	return trash_file_name;	
}

char *try_file_name(char *name_to_try, int number_to_try) {
	char *new_name_to_try;
	new_name_to_try = calloc((strlen(name_to_try) + sizeof(int) + 1), sizeof(char));
	if(new_name_to_try == NULL) {
		printf("could not allocate memory\n");
		exit(1);
	}
	
	sprintf(new_name_to_try, "%s.%d", name_to_try, number_to_try);

	if(access(new_name_to_try, F_OK) == 0) {
		return try_file_name(name_to_try, ++number_to_try);
	}
	else {
		free(name_to_try);
		return new_name_to_try;
	}
}

