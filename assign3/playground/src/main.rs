// Starter file provided to CSC 330, Summer 2026, Assignment 3
// Copyright Mike Zastre, UVic 2026.
//
// This echoes the functionality provided by the starter file in
// Haskell for the similar problem in Assignment 1.
//
// Therefore your task is to complete the functionality needed
// for the ParseNode datatype definition (i.e. completing
// the `enum` statement) as well as for operations on the
// datatype (i.e., completing the `eval` function on ParseNode.)
//

#[derive(Debug, PartialEq)]

// The `derive` above will help as the `Debug` trait will help
// produce useful output when trying to print a `ParseNode`.
//
// Similarly the `PartialEq` trait will provide default implementations
// of such opertions at `==` and `!=`.
//

enum ParseNode {
    Expr(Box<ParseNode>),
    Add(Box<ParseNode>, Box<ParseNode>),
    Sub(Box<ParseNode>, Box<ParseNode>),
    
    Term(Box<ParseNode>),
    Mult(Box<ParseNode>, Box<ParseNode>),
    Div(Box<ParseNode>, Box<ParseNode>),

    Factor(Box<ParseNode>),
    Power(Box<ParseNode>, Box<ParseNode>),
    
    Base(Box<ParseNode>),
    Number(i64),

}

impl ParseNode {
    fn eval(&self) -> i64 {
        match self {
            ParseNode::Expr(x) => x.eval(),
            ParseNode::Add(left, right) => left.eval() + right.eval(),
            ParseNode::Sub(left, right) => left.eval() - right.eval(),

            ParseNode::Term(x) => x.eval(),
            ParseNode::Mult(left, right) => left.eval() * right.eval(),
            ParseNode::Div(left, right) => left.eval() / right.eval(),
            
            ParseNode::Factor(x) => x.eval(),
            ParseNode::Power(left, right) => left.eval().pow(right.eval() as u32),
            
            ParseNode::Base(x) => x.eval(),
            ParseNode::Number(n) => *n,
        }
    }
}

fn main() {
    // Example expressions matching Haskell's parse_a, parse_b, parse_c, parse_d
    
    let parse_z = ParseNode::Number(42);
    println!("Result of parse_z: {}", parse_z.eval()); // Should print 42


    // Code for expression (a) in the assignment?
    
    //TEST
    /*let parse_a = ParseNode::Expr(
                    Box::new(ParseNode::Add(
                        Box::new(ParseNode::Term(
                            Box::new(ParseNode::Factor(
                                Box::new(ParseNode::Base(
                                        Box::new(ParseNode::Number(2)))))))))),
                        Box::new(ParseNode::Term(
                            Box::new(ParseNode::Factor(
                                Box::new(ParseNode::Base(
                                        Box::new(ParseNode::Number(3)))))))));*/
                                        
    let parse_a = ParseNode::Add(
                        Box::new(ParseNode::Expr(
                            Box::new(ParseNode::Number(2)))),
                        Box::new(ParseNode::Term(
                            Box::new(ParseNode::Mult(
                                    Box::new(ParseNode::Term(
                                        Box::new(ParseNode::Number(3)))),
                                    Box::new(ParseNode::Factor(
                                        Box::new(ParseNode::Number(4)))))))));
                        
    println!("Result of parse_a: {}", parse_a.eval());


    // Code for expression (b) in the assignment?
    
    let parse_b = ParseNode::Mult(
                    Box::new(ParseNode::Term(
                        Box::new(ParseNode::Add(
                            Box::new(ParseNode::Expr(
                                Box::new(ParseNode::Number(3)))),
                            Box::new(ParseNode::Term(
                                Box::new(ParseNode::Number(5)))))))),
                    Box::new(ParseNode::Factor(
                        Box::new(ParseNode::Sub(
                            Box::new(ParseNode::Expr(
                                Box::new(ParseNode::Number(7)))),
                            Box::new(ParseNode::Term(
                                Box::new(ParseNode::Number(2)))))))));
                                
    println!("Result of parse_b: {}", parse_b.eval());

    // Code for expression (c) in the assignment?
    
    let parse_c = ParseNode::Sub(
    Box::new(ParseNode::Expr(
        Box::new(ParseNode::Add(
            Box::new(ParseNode::Expr(
                Box::new(ParseNode::Number(4)))),
            Box::new(ParseNode::Term(
                Box::new(ParseNode::Mult(
                    Box::new(ParseNode::Term(
                        Box::new(ParseNode::Number(6)))),
                    Box::new(ParseNode::Factor(
                        Box::new(ParseNode::Power(
                            Box::new(ParseNode::Base(
                                Box::new(ParseNode::Number(2)))),
                            Box::new(ParseNode::Factor(
                                Box::new(ParseNode::Number(3)))))))))))))))),
    Box::new(ParseNode::Term(
        Box::new(ParseNode::Div(
            Box::new(ParseNode::Term(
                Box::new(ParseNode::Number(5)))),
            Box::new(ParseNode::Factor(
                Box::new(ParseNode::Add(
                    Box::new(ParseNode::Expr(
                        Box::new(ParseNode::Number(1)))),
                    Box::new(ParseNode::Term(
                        Box::new(ParseNode::Number(1)))))))))))));
                        
    println!("Result of parse_c: {}", parse_c.eval());

    // Code for expression (d) in the assignment?
    
    let parse_d = ParseNode::Sub(
    Box::new(ParseNode::Expr(
        Box::new(ParseNode::Add(
            Box::new(ParseNode::Expr(
                Box::new(ParseNode::Number(14)))),
            Box::new(ParseNode::Term(
                Box::new(ParseNode::Mult(
                    Box::new(ParseNode::Term(
                        Box::new(ParseNode::Number(26)))),
                    Box::new(ParseNode::Factor(
                        Box::new(ParseNode::Power(
                            Box::new(ParseNode::Base(
                                Box::new(ParseNode::Number(12)))),
                            Box::new(ParseNode::Factor(
                                Box::new(ParseNode::Number(3)))))))))))))))),
    Box::new(ParseNode::Term(
        Box::new(ParseNode::Div(
            Box::new(ParseNode::Term(
                Box::new(ParseNode::Number(55)))),
            Box::new(ParseNode::Factor(
                Box::new(ParseNode::Add(
                    Box::new(ParseNode::Expr(
                        Box::new(ParseNode::Number(10)))),
                    Box::new(ParseNode::Term(
                        Box::new(ParseNode::Number(11)))))))))))));
    
    println!("Result of parse_d: {}", parse_d.eval());
}
