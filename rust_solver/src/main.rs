
use std::env;
use std::fs;



pub fn pretty_print(grid: &Vec<u8>){
    let mut row = 0;
    let mut col = 0;

    while row < 9 {
        if row % 3 == 0{
            println!("");
        }
        let i = row * 9;
        col = 0;
        while col < 9{
            if col  %3 == 0{ print!("|");}
            print!("{}",{&grid[i+col]});
            col += 1;
        } 
        println!("|");
        row += 1;
    }
}

pub fn check(grid: &Vec<u8>, index: usize) -> bool{

    let mut to_check: Vec<u8> = Vec::new();
    let row = index / 9;
    let col = index % 9;
    let fieldr = row /3;
    let fieldc = col /3;

    for i in 0..9 {
        if i != col {
            to_check.push(grid[9*row + i]);
        }
        if i != row {
            to_check.push(grid[i * 9 + col]);
        }
    }

    for i in 0..3 {
        let k1 = fieldr*27 + 9*i;
        for j in 0..3 {
            let k2 = fieldc * 3 + j + k1;
            if k2 != index {
                to_check.push(grid[k2]);
            }
        }
    }


    if to_check.contains(&grid[*&index]) { return false; }

    true
}

pub fn solve(mut grid: &mut Vec<u8>, free_field: &Vec<usize>, mut index: usize ) -> bool{

    

    while index < 81 && grid[index] != 0 {
        index += 1;
    }
    
    if index == 81 { return true;}

    for digit in 0..10 {
        grid[index] = digit;

        if !check(&grid, index) { continue; };

        if solve(&mut grid,&free_field, index+1){
            return true;
        }

    }

    grid[index] = 0;
    false

    


    


}  

fn main() {
    let args: Vec<String> = env::args().collect();
    let file_path = &args[1]; //args 0 ist die datei

    let bytes = fs::read(file_path)
        .expect("Fehler beim Lesen der Datei.");

    let mut grid: Vec<u8> = bytes
        .into_iter()                                        // Gehe durch jedes Byte
        .filter(|&b| (b >= b'0' && b <= b'9') || b == b'.') // Filter: '0'-'9' oder '.'
        .take(81)
        .map(|b| {
            if b == b'.' { 
                0 
            } else { 
                b - b'0' // In Rust: Byte-Arithmetik (wie in C)
            }
        })
        .collect();
    
    if grid.len() != 81 {
        println!("Sudoku nicht richtig geprinted!");
        panic!();
    }

    pretty_print(&grid); //print unsolved



    let mut free_field: Vec<usize> = Vec::new(); //get free fields
    let mut index: usize = 0;
    while index < 81 {
        if grid[index] == 0{
            free_field.push(index);
        }
        index += 1;
    }

    index = 0;
    solve(&mut grid, &free_field, index);
    println!("---------------------------------------------------------------------");
    pretty_print(&grid);


}
