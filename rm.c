//Kevin Kell
//Distributed Systems Project 1
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <ftw.h>

#define BUFF_SIZE 512
#define DIR_DEPTH 25

void rm_force();
void rm_recursive();
void rm_non_recursive();
char *name_trash_file();
char *try_file_name(char* name_to_try, int number_to_try);
void copy(const char *to_be_copied, const char *destination);
void copy_directory(const char *to_be_copied, const char *destination);
int tree_function(const char *path, const struct stat *stat_buffer, int typeflag, struct FTW *ftw_buffer);
int delete_directory_tree(const char *path, const struct stat *stat_buffer, int typeflag, struct FTW *ftw_buffer);
void crop_file_path(char *path, int depth_to_keep);
void usage(void);

int f_flag = 0, h_flag = 0, r_flag = 0;
char *file_name;
char *dir_name;
int current_option;
char *trash_location;

int main(int argc, char *argv[]) {
	int j = 0;
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
	if(h_flag != 0) {
		usage();
		return 0;
	}

	trash_location = getenv("TRASH");
	if(trash_location == NULL) {
		printf("Please set TRASH environment variable\n");
		exit(1);
	}
	else{
		fprintf(stderr, "TRASH: %s \n", trash_location);
	}

	for(j=optind; j<argc; j++) {
		int length = strlen(argv[j]) + 1;
		char file_arg_basename[length];
		char file_arg_dirname[length];
		
		strcpy(file_arg_basename, argv[j]);
		strcpy(file_arg_dirname, argv[j]);
		file_name = malloc(length);
		strcpy(file_name, basename(file_arg_basename));

		dir_name = malloc(length);
		strcpy(dir_name, dirname(file_arg_dirname));

		if(chdir(dir_name) != 0) {
			perror("chdir");
			exit(1);
		}

		if(r_flag != 0) {
			rm_recursive();
		}
		if(f_flag != 0) {
			rm_force();
		}
		else {
			rm_non_recursive();
		}

		free(file_name);
		free(dir_name);
	}

	return 0;
}

void rm_force() {
	struct stat old_file_stat;
	if(stat(file_name, &old_file_stat) != 0) {
		perror("stat");
		exit(1);
	}

	if(remove(file_name) != 0) {
		if((errno == ENOTEMPTY || errno == EEXIST) && (f_flag != 0)) {
			if(nftw(file_name, delete_directory_tree, DIR_DEPTH, FTW_CHDIR | FTW_DEPTH) != 0) {
				perror("nftw");
				exit(1);
			}
		}
		else {
			perror("remove");
			exit(1);
		}
	}
}

void rm_recursive() {
	struct stat old_file_stat;
	if(stat(file_name, &old_file_stat) != 0) {
		perror("stat");
		exit(1);
	}

	if(S_ISREG(old_file_stat.st_mode)) {
		rm_non_recursive();
		return;
	}

	if(nftw(file_name, tree_function, DIR_DEPTH, FTW_CHDIR | 0) != 0) {
		perror("nftw");
		exit(1);
	}

	if(nftw(file_name, delete_directory_tree, DIR_DEPTH, FTW_CHDIR | FTW_DEPTH) != 0) {
		perror("nftw");
		exit(1);
	}
}

void rm_non_recursive() {
	int result;
	char *destination;
	struct stat old_file_stat;

	if(stat(file_name, &old_file_stat) != 0) {
		perror("stat");
		exit(1);
	}		

	destination = name_trash_file();

	printf("destination: %s \n", destination);

	result = rename(file_name, destination);
	if(result != 0) {
		if(errno == EXDEV) {
			struct stat old_file_stat;
			if(stat(file_name, &old_file_stat) != 0) {
				perror("stat");
				exit(1);
			}

			if(S_ISDIR(old_file_stat.st_mode)) {
				copy_directory(file_name, destination);
				printf("Moved %s to trash\n", file_name);
				free(destination);
			}
			else {
				copy(file_name, destination);
				printf("Moved %s to trash\n", file_name);	
				free(destination);
			}
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

void copy(const char *to_be_copied, const char *destination) {
	char buffer[BUFF_SIZE];
	int old_file, new_file;
	ssize_t bytes_read, bytes_written;
	struct stat old_file_stat;
	struct utimbuf old_file_times;

	if(stat(to_be_copied, &old_file_stat) != 0) {
		perror("stat");
		exit(1);
	}

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

	if(fchmod(new_file, old_file_stat.st_mode) != 0) {
		perror("fchmod");
		exit(1);
	}

	if(fchown(new_file, old_file_stat.st_uid, old_file_stat.st_gid) != 0) {
		perror("fchown");
	}

	old_file_times.actime = old_file_stat.st_atime;
	old_file_times.modtime = old_file_stat.st_mtime;
	if(utime(destination, &old_file_times) != 0) {
		perror("utime");
		exit(1);
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
	}
}

void copy_directory(const char *to_be_copied, const char *destination) {
	struct stat old_file_stat;
	if(stat(to_be_copied, &old_file_stat) != 0) {
		perror("stat");
		exit(1);
	}

	if((r_flag == 0) && (remove(to_be_copied) != 0)) {
		perror("remove old directory");
		exit(1);
	}	

	if(mkdir(destination, old_file_stat.st_mode)) {
		perror("mkdir");
		exit(1);
	}
}

int tree_function(const char *path, const struct stat *stat_buffer, int typeflag, struct FTW *ftw_buffer) {
	char path_copy[strlen(path) + 1];
	char *working_directory;

	working_directory = getcwd(NULL, 0);
	strcpy(path_copy, path);

	if(typeflag == FTW_F) {
		char source_file_name[strlen(working_directory) + strlen(path) + 1];
		char destination_file_name[strlen(getenv("TRASH")) + strlen(path) + 1];

		strcpy(source_file_name, working_directory);
		strcat(source_file_name, "/");
		strcat(source_file_name, basename(path_copy));

		strcpy(destination_file_name, getenv("TRASH"));
		strcat(destination_file_name, path);

		copy(source_file_name, destination_file_name);
	}
	else {
		char destination_directory[strlen(getenv("TRASH")) + strlen(path) + 1];

		strcpy(destination_directory, getenv("TRASH"));
		strcat(destination_directory, path);

		copy_directory(working_directory, destination_directory);
	}
	return 0;
}

int delete_directory_tree(const char *path, const struct stat *stat_buffer, int typeflag, struct FTW *ftw_buffer) {
	char *working_directory;
	char path_copy[strlen(path) + 1];

	strcpy(path_copy, path);
	working_directory = getcwd(NULL, 0);

	if(typeflag == FTW_F) {
		remove(basename(path_copy));
	}
	else {
		if(rmdir(working_directory) != 0) {
			perror("rmdir");
			exit(1);
		}
	}

	return 0;
}

void usage(void) {
	fprintf(stderr, "usage: rm [-r] [-f] [h] [<args>]\n");
	fprintf(stderr, "\t-r <files | directories> removes files and directories recursively\n");
	fprintf(stderr, "\t-f <files | directories> removes files and directories without moving to TRASH\n");
	fprintf(stderr, "\t-h prints usage\n");
}

void crop_file_path(char *path, int depth_to_keep) {
	

	int path_length = strlen(path);
	char directory_levels[DIR_DEPTH][path_length + 1];
	char cropped_path[path_length + 1];
	char *token;
	int i = 0;

	token = strtok(path, "/");
	while(token != NULL) {
		char *pointer;

		strcpy(directory_levels[i], token);
		pointer = directory_levels[i];
		i++;
		token = strtok(NULL, "/");
	}

	memset(cropped_path, 0, path_length + 1);
	for(i=depth_to_keep; i>=0; i--){
		char tmp[path_length + 1];
		sprintf(tmp, "/%s", directory_levels[i]);
		strcat(cropped_path, tmp);
	}

	strcpy(path, cropped_path);
}
