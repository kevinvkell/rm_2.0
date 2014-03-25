//Kevin Kell
//Distributed Systems Project 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <fcntl.h>

#define BUFF_SIZE 512
void print_info(char *message);
void rm_recursive();
void rm_non_recursive();
char *name_trash_file();
char *try_file_name(char* name_to_try, int number_to_try);
void copy(char *to_be_copied, char *destination);

int f_flag = 0, h_flag = 0, r_flag = 0;
char *file_name;
char *dir_name;
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
		file_name = basename(argv[optind]);
		dir_name = dirname(argv[optind]);
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

	printf("f_flag: %d \nh_flag: %d \nr_flag: %d \ndirname: %s \nfilename: %s \n", f_flag, h_flag, r_flag, dir_name, file_name);

	if(chdir(dir_name) != 0) {
		perror("chdir");
		exit(1);
	}

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
	if(result != 0) {
		if(errno == EXDEV) {
			copy(file_name, destination);
			printf("Moved %s to trash\n", file_name);	
			free(destination);
		}
		else{
			free(destination);	
			perror("rename");
			exit(1);
		}
	}
	else {
			free(destination);
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

void copy(char *to_be_copied, char *destination) {
	char buffer[BUFF_SIZE];
	int old_file, new_file;
	ssize_t bytes_read, bytes_written;

	old_file = open(to_be_copied, O_RDONLY | O_NONBLOCK);
	if (old_file < 0) {
		perror("open old file");
		exit(1);
	}

	new_file = open(destination, O_WRONLY | O_CREAT | O_EXCL |O_NONBLOCK);
	if (new_file < 0) {
		perror("open new file");
		exit(1);
	}

	while((bytes_read = read(old_file, buffer, BUFF_SIZE)) > 0) {
		bytes_written = write(new_file, buffer, bytes_read);
		if(bytes_written != bytes_read) {
			printf("failed to write properly\n");
			exit(1);
		}
	}

	if(close(old_file) != 0) {
		perror("close old file");
		exit(1);
	}
	if(close(new_file) != 0) {
		perror("close new file");
		exit(1);
	}

	if(remove(to_be_copied) != 0) {
		perror("remove old file");
		exit(1);
	}
}
