#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <fcntl.h>
#include "proj4.h"

void initializeGrid(grid * g, char * filename) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		printf("File cannot open for reading!\n");
	}
	char buffer[2];
	int n = -1;
	while (buffer[0] != '\n') {
		read(fd, buffer, 1);
		n++;
	}
	lseek(fd, 0, SEEK_SET);
	(*g).n = n;
	(*g).p = malloc(sizeof(char**) * n);
	for (int i = 0; i < n; i++) {
		*((*g).p + i) = malloc(sizeof(char*) * n);
		read(fd, *((*g).p + i), n);
		lseek(fd, 1, SEEK_CUR);
	}
	close(fd);
}

void diagonalSums(grid * input, unsigned long s, grid * output, int t) {
	int n = (*input).n;
	(*output).n = n;
	(*output).p = malloc(sizeof(char**) * n);
	for (int i = 0; i < n; i++) {
		*((*output).p + i) = malloc(sizeof(char*) * n);
		for (int j = 0; j < n; j++) {
			*(*((*output).p + i) + j) = '0';
		}
	}

	p_args p2a;
	p_args p3a;
	pthread_t pthread2;
	pthread_t pthread3;

	p2a.input = input;
	p2a.output = output;
	p2a.inc = 2;
	p2a.target = s;
	p2a.start = 1;

	p3a.input = input;
	p3a.output = output;
	p3a.inc = 3;
	p3a.target = s;
	p3a.start = 2;

	if (t == 1) {
		calculate(input, output, 1, s, 0);
	} else if (t == 2) {
		calculate(input, output, 2, s, 0);
		if (pthread_create(&pthread2, NULL, threadwork, &p2a) != 0) {
			printf("Error creating thread!");
		}
		if (pthread_join(pthread2, NULL) != 0) {
			printf("Error joining pthread2 with first");
		}
	} else if (t == 3) {
		p2a.inc = 3;
		calculate(input, output, 3, s, 0);
		if (pthread_create(&pthread2, NULL, threadwork, &p2a) != 0) {
			printf("Error creating thread!");
		}
		if (pthread_create(&pthread3, NULL, threadwork, &p3a) != 0) {
			printf("Error creating thread!");
		}
		if (pthread_join(pthread2, NULL) != 0) {
			printf("Error joining pthread2 with first");
		}
		if (pthread_join(pthread3, NULL) != 0) {
			printf("Error joining pthread2 with first");
		}
	} else {
		printf("Usage: 1 <= t <= 3\n");
	}

}

void * threadwork(void * args) {
	p_args * pArgs = (p_args *) args;
	calculate(pArgs->input, pArgs->output, pArgs->inc, pArgs->target, pArgs->start);
	return NULL;
}

void calculate(grid * input, grid * output, int inc, int target, int start) {
	int n = (*input).n;
	for (int i = start; i < n; i += inc) {
		for (int j = 0; j < n; j++) {
			dfs(input, output, target, i, j, 1);
			dfs(input, output, target, i, j, -1);
		}
	}
}
bool dfs(grid * input, grid * output, int target, int i, int j, int inc) {
	int n = (*input).n;
	if (i >= n || i < 0 || j >= n || j < 0) {
		return false;
	}
	if (target < 0) {
		return false;
	}
	int k = *(*((*input).p + i) + j) - '0';
	if (target - k == 0) {
		*(*((*output).p + i) + j) = *(*((*input).p + i) + j);
		return true;
	}
	if (dfs(input, output, target - k, i + 1, j + inc, inc)) {
		*(*((*output).p + i) + j) = *(*((*input).p + i) + j);
		return true;
	}
	return false;
}

void writeGrid(grid * g, char * fileName) {
	int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	int n = (*g).n;
	if (fd < 0) {
		printf("Failed to open for writing\n");
	}
	char * nl = "\n";
	for (int i = 0; i < n; i++) {
		write(fd, *((*g).p + i), n);
		write(fd, nl, 1);
	}
	close(fd);
}

void freeGrid(grid * g) {
	for (int i = 0; i < (*g).n; i++) {
		free(*((*g).p + i));
	}
	free((*g).p);

}
