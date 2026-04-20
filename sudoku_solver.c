#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define RSIZE 9
#define CSIZE 9
#define BSIZE 81

static int unit[27][9];



int check(int* grid) {
    for (int digit = 1; digit <= 9; digit++) {
        // Zeilen
        for (int row = 0; row < 9; row++) {
            int count = 0;
            for (int col = 0; col < 9; col++)
                if (grid[row*9 + col] == digit) count++;
            if (count != 1) return 0;
        }
        // Spalten
        for (int col = 0; col < 9; col++) {
            int count = 0;
            for (int row = 0; row < 9; row++)
                if (grid[row*9 + col] == digit) count++;
            if (count != 1) return 0;
        }
        // Boxen
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                int count = 0;
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (grid[x*27 + y*3 + i*9 + j] == digit) count++;
                if (count != 1) return 0;
            }
        }
    }
    return 1;
}

void pretty_print(int* grid) {
    printf("\n");
    for (int row = 0; row < 9; row++) {
        if (row % 3 == 0 && row != 0)
            printf("------+-------+------\n");
        for (int col = 0; col < 9; col++) {
            if (col % 3 == 0 && col != 0)
                printf("| ");
            int v = grid[row*9 + col];
            if (v == 0) printf(". ");
            else        printf("%d ", v);
        }
        printf("\n");
    }
    printf("\n");
}

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

int clear(int index, int digit, uint16_t* notes){
    uint16_t mask = ~(1u << (digit - 1));
    int row = index / 9;
    int col = index % 9;

    // Zeile
    for(int i = 0; i < 9; i++){
        int k = row*9 + i;
        if(k != index) notes[k] &= mask;
    }

    // Spalte
    for(int i = 0; i < 9; i++){
        int k = i*9 + col;
        if(k != index) notes[k] &= mask;
    }

    // Box
    int rfield = row / 3;
    int cfield = col / 3;
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            int k = rfield*27 + cfield*3 + i*9 + j;
            if(k != index) notes[k] &= mask;
        }
    }

    return 0;
}

int set_notes(int* grid, uint16_t* notes){

    for(int i = 0; i<BSIZE; i++){
        notes[i] = 0x1FF;
    }

	for(int i = 0; i<BSIZE; i++){
		if(grid[i] != 0){

            notes[i] = 1 << (grid[i]-1); //set note
			clear(i,grid[i], notes); //clear all other interfering notes

		}
	}
    return 0;
}

int naked_single(int* grid, uint16_t* notes){
    int changed = 0;
    for(int i = 0; i<BSIZE; i++){
        uint16_t v = notes[i];
        if(grid[i] == 0 && v != 0 && (v & (v -1)) == 0 ){
            grid[i] = __builtin_ctz(notes[i]) + 1;
            clear(i,grid[i],notes);
            changed = 1;
        }
    }
    return changed;
}

int hidden_single(int* grid, uint16_t* notes){
    int changed = 0;
    for(int digit = 0 ; digit < 9; digit++){
        uint16_t mask = 1 << digit;

        //rows
        for(int row = 0; row<9; row++){
            int count = 0; int target = -1;
            for(int col = 0; col<9; col++){
                if(notes[row*9+col] & mask){
                    count++;
                    target = row*9 + col;
                }
            }
            if(count == 1 && grid[target] == 0){
                grid[target] = digit + 1;
                notes[target] = mask;
                clear(target, grid[target], notes);
                changed = 1;
            }
        }

        //columns
        for(int col = 0; col<9; col++){
            int count = 0; int target = -1;
            for(int row = 0; row<9;row++){
                if(notes[row*9+col] & mask){
                    count++;
                    target = row*9 + col;
                }
            }
            if(count == 1 && grid[target] == 0){
                grid[target] = digit + 1;
                notes[target] = mask;
                clear(target, grid[target], notes);
                changed = 1;
            }
        }

        //Boxes
        for(int x = 0; x<3; x++){
            for(int y = 0; y<3; y++){
                int count = 0; int target = -1;
                for(int i = 0; i<3; i++){
                    for(int j = 0; j<3; j++){
                        int index = x*27 + y*3 + i*9 + j;
                        if(notes[index] & mask){
                            count++;
                            target = index;
                        }
                    }
                }
                if(count == 1 && grid[target] == 0){
                    grid[target] = digit + 1;
                    notes[target] = mask;
                    clear(target, grid[target], notes);
                    changed = 1;
                }
            }
        }



    }
    return changed;

}

int naked_pair(int* grid, uint16_t* notes){
    int changed = 0;
    //rows
    for(int row = 0; row<RSIZE; row++){
        for(int col1 = 0; col1<CSIZE; col1++){
            if(__builtin_popcount(notes[row*RSIZE + col1]) == 2){
                for(int col2 = col1+1; col2<CSIZE; col2++){
                    if(notes[row*RSIZE + col2] == notes[row*RSIZE + col1]){
                        //Naked Pair found -> clear row
                        for(int i = 0; i<RSIZE;i++){
                            if(i != col1 && i != col2){
                                uint16_t old = notes[row*RSIZE + i];
                                notes[row*RSIZE + i] &= ~(notes[row*RSIZE + col1]);
                                if (notes[row*RSIZE + i] != old) changed = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    //cols
    for(int col = 0; col<CSIZE; col++){
        for(int row1 = 0; row1<RSIZE; row1++){
            if(__builtin_popcount(notes[row1*RSIZE + col]) == 2){
                for(int row2 = row1+1; row2<RSIZE; row2++){
                    if(notes[row1*RSIZE + col] == notes[row2*RSIZE + col]){
                        //Naked Pair found -> clear col
                        for(int i = 0; i<RSIZE;i++){
                            if(i != row1 && i != row2){
                                notes[i*RSIZE + col] &= ~(notes[row1*RSIZE + col]);
                                changed = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    //boxes
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            int idx[9];
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    idx[i*3+j] = x*27 + y*3 + i*9 + j;

            for (int a = 0; a < 9; a++) {
                if (__builtin_popcount(notes[idx[a]]) != 2) continue;
                for (int b = a+1; b < 9; b++) {
                    if (notes[idx[b]] != notes[idx[a]]) continue;
                    uint16_t mask = ~notes[idx[a]];
                    for (int i = 0; i < 9; i++) {
                        if (i != a && i != b) {
                            notes[idx[i]] &= mask;
                            changed = 1;
                        }
                    }
                }
            }
        }
    }

    return changed;
}

int hidden_pair(int* grid, uint16_t* notes) {
    int changed = 0;

    int units[27][9];
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            units[r][c] = r*9 + c;
    for (int c = 0; c < 9; c++)
        for (int r = 0; r < 9; r++)
            units[9+c][r] = r*9 + c;
    for (int b = 0; b < 9; b++) {
        int frow = (b/3)*3, fcol = (b%3)*3;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                units[18+b][i*3+j] = (frow+i)*9 + (fcol+j);
    }

    for (int u = 0; u < 27; u++) {
        for (int d1 = 0; d1 < 9; d1++) {
            uint16_t m1 = 1u << d1;
            for (int d2 = d1+1; d2 < 9; d2++) {
                uint16_t m2 = 1u << d2;
                uint16_t pair = m1 | m2;

                // Felder finden die d1 oder d2 enthalten
                int count = 0, targets[2];
                for (int k = 0; k < 9; k++) {
                    if (notes[units[u][k]] & pair) {
                        if (count < 2) targets[count] = units[u][k];
                        count++;
                    }
                }

                // Genau 2 Felder -> Hidden Pair
                if (count == 2) {
                    for (int t = 0; t < 2; t++) {
                        uint16_t old = notes[targets[t]];
                        notes[targets[t]] &= pair;
                        if (notes[targets[t]] != old) changed = 1;
                    }
                }
            }
        }
    }
    return changed;
}

int naked_triple(int* grid, uint16_t* notes) {
    int changed = 0;

    // Zeilen
    for (int row = 0; row < 9; row++) {
        for (int col1 = 0; col1 < 9; col1++) {
            if (__builtin_popcount(notes[row*9+col1]) > 3) continue;
            for (int col2 = col1+1; col2 < 9; col2++) {
                if (__builtin_popcount(notes[row*9+col2]) > 3) continue;
                for (int col3 = col2+1; col3 < 9; col3++) {
                    uint16_t uni = notes[row*9+col1] | notes[row*9+col2] | notes[row*9+col3];
                    if (__builtin_popcount(uni) != 3) continue;
                    uint16_t mask = ~uni;
                    for (int i = 0; i < 9; i++) {
                        if (i != col1 && i != col2 && i != col3) {
                            notes[row*9+i] &= mask;
                            changed = 1;
                        }
                    }
                }
            }
        }
    }

    // Spalten
    for (int col = 0; col < 9; col++) {
        for (int row1 = 0; row1 < 9; row1++) {
            if (__builtin_popcount(notes[row1*9+col]) > 3) continue;
            for (int row2 = row1+1; row2 < 9; row2++) {
                if (__builtin_popcount(notes[row2*9+col]) > 3) continue;
                for (int row3 = row2+1; row3 < 9; row3++) {
                    uint16_t uni = notes[row1*9+col] | notes[row2*9+col] | notes[row3*9+col];
                    if (__builtin_popcount(uni) != 3) continue;
                    uint16_t mask = ~uni;
                    for (int i = 0; i < 9; i++) {
                        if (i != row1 && i != row2 && i != row3) {
                            notes[i*9+col] &= mask;
                            changed = 1;
                        }
                    }
                }
            }
        }
    }

    // Boxen
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            int idx[9];
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    idx[i*3+j] = x*27 + y*3 + i*9 + j;

            for (int a = 0; a < 9; a++) {
                if (__builtin_popcount(notes[idx[a]]) > 3) continue;
                for (int b = a+1; b < 9; b++) {
                    if (__builtin_popcount(notes[idx[b]]) > 3) continue;
                    for (int c = b+1; c < 9; c++) {
                        uint16_t uni = notes[idx[a]] | notes[idx[b]] | notes[idx[c]];
                        if (__builtin_popcount(uni) != 3) continue;
                        uint16_t mask = ~uni;
                        for (int i = 0; i < 9; i++) {
                            if (i != a && i != b && i != c) {
                                notes[idx[i]] &= mask;
                                changed = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    return changed;
}

int hidden_triple(int* grid, uint16_t* notes) {
    int changed = 0;

    int units[27][9];
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            units[r][c] = r*9 + c;
    for (int c = 0; c < 9; c++)
        for (int r = 0; r < 9; r++)
            units[9+c][r] = r*9 + c;
    for (int b = 0; b < 9; b++) {
        int frow = (b/3)*3, fcol = (b%3)*3;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                units[18+b][i*3+j] = (frow+i)*9 + (fcol+j);
    }

    for (int u = 0; u < 27; u++) {
        for (int d1 = 0; d1 < 9; d1++) {
            uint16_t m1 = 1u << d1;
            for (int d2 = d1+1; d2 < 9; d2++) {
                uint16_t m2 = 1u << d2;
                for (int d3 = d2+1; d3 < 9; d3++) {
                    uint16_t m3 = 1u << d3;
                    uint16_t triple = m1 | m2 | m3;

                    // Felder finden die mindestens eine der 3 Ziffern enthalten
                    int count = 0, targets[3];
                    for (int k = 0; k < 9; k++) {
                        if (notes[units[u][k]] & triple) {
                            if (count < 3) targets[count] = units[u][k];
                            count++;
                        }
                    }

                    // Genau 3 Felder -> Hidden Triple
                    if (count == 3) {
                        for (int t = 0; t < 3; t++) {
                            uint16_t old = notes[targets[t]];
                            notes[targets[t]] &= triple;
                            if (notes[targets[t]] != old) changed = 1;
                        }
                    }
                }
            }
        }
    }
    return changed;
}

int naked_quad(int* grid, uint16_t* notes) {
    int changed = 0;

    int units[27][9];
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            units[r][c] = r*9 + c;
    for (int c = 0; c < 9; c++)
        for (int r = 0; r < 9; r++)
            units[9+c][r] = r*9 + c;
    for (int b = 0; b < 9; b++) {
        int frow = (b/3)*3, fcol = (b%3)*3;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                units[18+b][i*3+j] = (frow+i)*9 + (fcol+j);
    }

    for (int u = 0; u < 27; u++) {
        for (int a = 0; a < 9; a++) {
            if (__builtin_popcount(notes[units[u][a]]) > 4) continue;
            for (int b = a+1; b < 9; b++) {
                if (__builtin_popcount(notes[units[u][b]]) > 4) continue;
                for (int c = b+1; c < 9; c++) {
                    if (__builtin_popcount(notes[units[u][c]]) > 4) continue;
                    for (int d = c+1; d < 9; d++) {
                        uint16_t uni = notes[units[u][a]] | notes[units[u][b]]
                                     | notes[units[u][c]] | notes[units[u][d]];
                        if (__builtin_popcount(uni) != 4) continue;
                        uint16_t mask = ~uni;
                        for (int i = 0; i < 9; i++) {
                            if (i != a && i != b && i != c && i != d) {
                                notes[units[u][i]] &= mask;
                                changed = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return changed;
}

int hidden_quad(int* grid, uint16_t* notes) {
    int changed = 0;

    int units[27][9];
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            units[r][c] = r*9 + c;
    for (int c = 0; c < 9; c++)
        for (int r = 0; r < 9; r++)
            units[9+c][r] = r*9 + c;
    for (int b = 0; b < 9; b++) {
        int frow = (b/3)*3, fcol = (b%3)*3;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                units[18+b][i*3+j] = (frow+i)*9 + (fcol+j);
    }

    for (int u = 0; u < 27; u++) {
        for (int d1 = 0; d1 < 9; d1++) {
            uint16_t m1 = 1u << d1;
            for (int d2 = d1+1; d2 < 9; d2++) {
                uint16_t m2 = 1u << d2;
                for (int d3 = d2+1; d3 < 9; d3++) {
                    uint16_t m3 = 1u << d3;
                    for (int d4 = d3+1; d4 < 9; d4++) {
                        uint16_t m4 = 1u << d4;
                        uint16_t quad = m1 | m2 | m3 | m4;

                        // Felder finden die mindestens eine der 4 Ziffern enthalten
                        int count = 0, targets[4];
                        for (int k = 0; k < 9; k++) {
                            if (notes[units[u][k]] & quad) {
                                if (count < 4) targets[count] = units[u][k];
                                count++;
                            }
                        }

                        // Genau 4 Felder -> Hidden Quad
                        if (count == 4) {
                            for (int t = 0; t < 4; t++) {
                                uint16_t old = notes[targets[t]];
                                notes[targets[t]] &= quad;
                                if (notes[targets[t]] != old) changed = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return changed;
}

bool check_fb(int i, int*grid){
    int row = i/9;
    int col = i%9;
    for(int j = 0; j<RSIZE; j++){
        int index = row*9+j;
        if(index != i && grid[index] == grid[i]) return false;
    }
    for(int j = 0; j<RSIZE; j++){
        int index = j*9+col;
        if(index != i && grid[index] == grid[i]) return false;
    }
    int x = row / 3;
    int y = col / 3;
    for(int a = 0; a<3; a++){
        for(int b = 0; b<3; b++){
            int index = 27*x + 3*y + 9*a + b;
            if(index != i && grid[index] == grid[i]) return false;
        }
    }
    return true;
}

bool fallback(int* grid) {
    for(int i = 0; i < BSIZE; i++) {
        if(grid[i] == 0) {
            for(int digit = 1; digit < 10; digit++) {
                grid[i] = digit;
                if(check_fb(i, grid)) {
                    if(fallback(grid)) return true;
                }
            }
            grid[i] = 0;  
            return false;
        }
    }
    return true; 
}



bool notFull(int* grid){
    for(int i = 0; i<BSIZE; i++){
        if(grid[i] == 0){
            return true;
        }
    }
    return false;
}

int* _solve(int* grid, uint16_t* notes){
	
	set_notes(grid,notes);

    int iter = 0;
	while(notFull(grid)){
        iter++;
        if(naked_single(grid,notes)) continue;
        if(hidden_single(grid,notes)) continue;

        if(naked_pair(grid,notes))continue;
        hidden_pair(grid,notes);


        
        if(iter > 80){
            printf("iter bigger than 1000 -> break\n");
            fallback(grid);
            break;
        }
	}

    return grid;

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

    int* (*solver_ptr) (int*,uint16_t* );
    solver_ptr = &_solve;
    

    // correctness
    int *grid1 = read_sudoku(path);
    printf("Original:\n");
    pretty_print(grid1);
    uint16_t* notes1 = malloc(81 * sizeof(uint16_t));
    _solve(grid1,notes1);
    int succ = check(grid1);
    pretty_print(grid1);
    free(grid1);
    free(notes1);

    // timing
    long long total_ns = 0;
    struct timespec start, end;

    int* grid_template = read_sudoku(path);
    int* grid = malloc(81 * sizeof(int));
    uint16_t* notes = malloc(81 * sizeof(uint16_t));    
    for (int i = 0; i<n; i++) {
        memcpy(grid, grid_template, 81 * sizeof(int));
        clock_gettime(CLOCK_MONOTONIC, &start);
        solver_ptr(grid,notes); // actual solving
        clock_gettime(CLOCK_MONOTONIC, &end);
        total_ns += ((end.tv_sec - start.tv_sec)*1000000000LL + end.tv_nsec - start.tv_nsec);
    }

    

    // print results
    printf("LANGUAGE:C\n");
    printf("SOLUTION:"); pretty_print(grid);
    printf((succ ? "SUCCESS:TRUE\n" : "SUCCESS:FALSE\n"));
    printf("MEAN_TIME_NS: %lld\n", total_ns/n);

    free(notes);
    free(grid);
    return 0;
}