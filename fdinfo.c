
#ifndef _FILE_OFFSET_BITS
# define _FILE_OFFSET_BITS 64
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

struct fdinfo {
	int fd;
	uintmax_t prev_pos;
	struct fdinfo * next;
};

int print_stats(pid_t pid, struct fdinfo ** fds);

void print_usage(FILE * out, char * argv[]) {
	fprintf(out, "Usage: %s <pid>\n", argv[0]);
}

int main(int argc, char * argv[]) {

	pid_t pid;
	struct fdinfo * fds = NULL;
	char * check;

	if (argc != 2) {
		print_usage(stderr, argv);
		return 1;
	}

	pid = (pid_t) strtol(argv[1], &check, 10);
	if (*check != '\0') {
		print_usage(stderr, argv);
		return 1;
	}

	for (;;) {
		int ret = print_stats(pid, &fds);
		if (ret) {
			return ret;
		}
		sleep(1);
	}

	return 0;
}

void format_size(char * buf, size_t buf_len, uintmax_t size) {

	const char * units[] = { "B  ", "KiB", "MiB", "GiB", "TiB" };
	unsigned int unit = 0;

	while (size > (1<<14) 
			&& unit+1 < sizeof(units)/sizeof(units[0])) {
		++unit;
		size >>= 10;
	}

	snprintf(buf, buf_len, "%5jd %s", size, units[unit]);
}

off_t get_filesize(const char * path) {
	struct stat st;
	memset(&st, 0, sizeof(st));
	if (!stat(path, &st)) {
		return st.st_size;
	} else {
		return -1;
	}
}

int print_stats(pid_t pid, struct fdinfo ** fds) {

	char path[128];
	DIR * dir;
	struct dirent * dent;

	snprintf(path, sizeof(path), "/proc/%d/fdinfo", pid);

	dir = opendir(path);
	if (!dir) {
		fprintf(stderr, "opendir(%s): %s\n", path, strerror(errno));
		return 1;
	}

	while ((dent = readdir(dir)) != NULL) {

		long fd_num;
		uintmax_t pos;
		char * p;
		FILE * fd;
		struct fdinfo * fdinfo;

		char filename[PATH_MAX];
		ssize_t filename_len;
		off_t filesize;
		char sizebuf1[64], sizebuf2[64], sizebuf3[64];
		
		fd_num = strtol(dent->d_name, &p, 10);
		if (*p != '\0') {
			continue;
		}

		snprintf(path, sizeof(path), "/proc/%d/fdinfo/%ld", pid, fd_num);

		fd = fopen(path, "r");
		if (!fd) {
			fprintf(stderr, "fopen(%s): %s\n", path, strerror(errno));
			return 1;
		}

		if (fscanf(fd, "%*s %jd", &pos) != 1) {
			perror("fscanf");
			return 1;
		}

		snprintf(path, sizeof(path), "/proc/%d/fd/%ld", pid, fd_num);

		filename_len = readlink(path, filename, sizeof(filename)-1);
		if (filename_len < 0) {
			snprintf(filename, sizeof(filename), "unknown");
			filename_len = strlen(filename);
			filesize = 0;
		} else {
			filename[filename_len] = '\0';
			filesize = get_filesize(filename);
			if (filesize < 0) {
				filesize = 0;
			}
		}

		for (fdinfo = *fds; fdinfo; fdinfo = fdinfo->next) {
			if (fdinfo->fd == fd_num) {
				break;
			}
		}
		if (!fdinfo) {
			fdinfo = calloc(1, sizeof(struct fdinfo));
			fdinfo->fd = fd_num;
			fdinfo->prev_pos = pos;
			fdinfo->next = *fds;
			*fds = fdinfo;
		}

		format_size(sizebuf1, sizeof(sizebuf1), pos);
		format_size(sizebuf2, sizeof(sizebuf2), pos - fdinfo->prev_pos);
		format_size(sizebuf3, sizeof(sizebuf3), filesize);
		printf("%5ld: %s / %s [%s / s] %s\n", 
				fd_num, sizebuf1, sizebuf3, sizebuf2, filename);

		fdinfo->prev_pos = pos;

		fclose(fd);
	}

	closedir(dir);

	return 0;
}

