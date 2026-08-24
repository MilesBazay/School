// Starter file provided to CSC 330, Summer 2026, Assignment 3
// Copyright Mike Zastre, UVic 2026.
//
// This echoes the functionality provided by the starter file in
// Haskell for the similar problem in Assignment 1.
//
// Therefore your task is to complete the functionality needed
// by `max_coverage()` -- and which will (perhaps) including writing
// other Rust functions in turn.
//

use std::fs::{File, read_to_string};
use std::env;
use std::io::{Write};


fn max_coverage(m: usize, denominations: Vec<u32>) -> (u32, Vec<u32>) {
    let denominations = denominations[1..].to_vec();
    let max_value = coverage_from(1, m, &denominations);

    (max_value, denominations)
}

fn coverage_from(amount: u32, max_stamps: usize, denominations: &[u32]) -> u32 {
    if can_make(amount, max_stamps, denominations) {
        coverage_from(amount + 1, max_stamps, denominations)
    } else {
        amount - 1
    }
}

fn can_make(amount: u32, stamps_left: usize, denominations: &[u32]) -> bool {
    if amount == 0 {
        true
    } else if stamps_left == 0 {
        false
    } else {
        try_stamps(amount, stamps_left, denominations)
    }
}

fn try_stamps(amount: u32, stamps_left: usize, denominations: &[u32]) -> bool {
    for &stamp in denominations {
        if stamp > amount {
            return false;
        }

        if can_make(amount - stamp, stamps_left - 1, denominations) {
            return true;
        }
    }

    false
}


fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() != 3 {
        eprintln!("Usage: {} <input file> <output file>", args[0]);
        return;
    }

    let contents: String = read_to_string(&args[1])
        .expect("Should have been able to read the file.");

    let mut lines = contents.lines();

    let mut output_file = File::create(&args[2])
        .expect("Failed to created output file");

    while let Some(size_line) = lines.next() {
        let m: usize = size_line.trim()
            .parse()
            .unwrap_or(0);

        if m == 0 {
            break;
        }
        
        if let Some(denoms_line) = lines.next() {
            let values: Vec<u32> = denoms_line
                .split_whitespace()
                .map(|s| s.parse().unwrap())
                .collect();
            
            let (max_value, denominations) = max_coverage(m, values);

            writeln!(output_file,
                    "max coverage = {} : {}",
                    max_value,
                    denominations
                        .iter()
                        .map(|n| n.to_string())
                        .collect::<Vec<String>>()
                        .join(" "))
                .expect("Failed to write to output file.");
        } else {
            eprintln!("Error: Expected a second line, but found none.");
            break;
        }
    }
}
