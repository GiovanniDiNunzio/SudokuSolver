#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#define RSIZE 9
#define BSIZE 81

int* read_sudoku(const char* path) {
    int* board = (int *)malloc(81 * sizeof(int));
    if (!board) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", path);
        free(board);
        return NULL;
    }

    int idx = 0;
    int c;
    while ((c = fgetc(file)) != EOF && idx < 81) {
        if (c >= '1' && c <= '9') {
            board[idx++] = c - '0';
        } else if (c == '.' || c == '0') {
            board[idx++] = 0;
        }
    }

    fclose(file);

    if (idx != 81) {
        fprintf(stderr, "Warning: Expected 81 cells, got %d\n", idx);
    }

    return board;
}



int* solve(char* path) {
    int* grid = read_sudoku(path);
    int result = _solve(grid);
    return grid;
}

int clear(int index, int digit, int* grid_copy){
	int m = BSIZE * (digit-1);
	//clear row
	int row = i/9;
	for(int i = 0; i<RSIZE; i++){
		int k = row*RSIZE + i;
		if(k != index){
			gird_copy[k + m]=1;
		}
	}

	//clear col
	int col = i%9;
	for(int i = 0; i<RSIZE; i++){
		int k = i*RSIZE + col;
		if(k != index){
			gird_copy[k + m]=1;
		}
	}

	//clear field
	int rfield = row/3;
	int cfield = col/3;

	for(int i = 0; i<3; i++){
		for(int j = 0; j<3; j++){
			int k = rfield *RSIZE + cfield + i + j;
			if(k != index){
				grid_copy[k + m]=1;
			}
		}
	}
	return 0;
}

int set_notes(int* grid, int* grid_copy){
	for(int i = 0; i<BSZIE; i++){
		if(grid[i] != 0){
			clear(i,grid[i], int* grid_copy);
		}
	}
}

bool unfinished(int * grid){
	for(int i= 0; i<BSIZE; i++){
		if(grid[i] == 0){
			return false;
		}
	}
	return true;
}


/*
grid_copy = 0 => possible, gird_copy = 1 => not possible
indexing:
gid_copy[k*81 + l*9 + m] => digit (k+1), row l, col m 
*/

int _solve(int* grid, int* grid_copy){
	
	set_notes(grid,grid_copy);

	while(unfinished(grid)){
		
	}

}



int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <path> <n> <version>\n", argv[0]);
        return 1;
    }

    char* path = argv[1];
    int n = atoi(argv[2]);
    int version = atoi(argv[3]);

    printf("path: %s\n", path);
    printf("n: %d\n", n);
    printf("version: %d\n", version);

    int (*solver_ptr) (int* );
    if (version == 0) solver_ptr = &_solve;
    else solver_ptr = &_solve2;

    // correctness
    int* sol = solve(path);
    int succ = check(sol);

    // timing
    long long total_ns = 0;
    struct timespec start, end;
    int* grid = read_sudoku(path);
    int* grid_copy = calloc(729 * sizeof(int));
    for (int i = 0; i<n; i++) {
        // copy grid
        copy(grid, grid_copy);
        clock_gettime(CLOCK_MONOTONIC, &start);
        solver_ptr(grid,grid_copy); // actual solving
        clock_gettime(CLOCK_MONOTONIC, &end);
        total_ns += ((end.tv_sec - start.tv_sec)*1000000000LL + end.tv_nsec - start.tv_nsec);

    }

    free(grid_copy);
    free(grid);

    // print results
    printf("LANGUAGE:C\n");
    printf("SOLUTION:"); pretty_print_sol(sol);
    printf((succ ? "SUCCESS:TRUE\n" : "SUCCESS:FALSE\n"));
    printf("MEAN_TIME_NS: %lld\n", total_ns/n);

    free(sol);
    return 0;
}