-- Some starter code for Part 1 of CSC 330 Summer 2026 A#1
-- 
-- Author: Mike Zastre
--
-- The copyright for this code is held by Mike Zastre. It is
-- not to be uploaded or otherwise accessed by any online service
-- or online-facilitated tool (such as Copilot or ChatGPT, although
-- this list is not exhaustive).



text_a = "2 + 3 * 4"
text_b = "(3 + 5) * (7 - 2)"
text_c = "(4 + 6 * (2 ^ 3)) - 5 / (1 + 1)"
text_d = "(14 + 26 * (12 ^ 3)) - 55 / (10 + 11)"


data ParseNode = 
        Expr ParseNode
      | Add ParseNode ParseNode
      | Sub ParseNode ParseNode
        
      | Term ParseNode
      | Mult ParseNode ParseNode
      | Div ParseNode ParseNode
        
      | Factor ParseNode
      | Power ParseNode ParseNode
        
      | Base ParseNode
      | Number ParseNode
        
      | Digit Integer
        deriving (Show, Eq)

parse_tree_a :: ParseNode
parse_tree_a =
    Add
        (Expr
            (Term
                (Factor
                    (Base
                        (Number
                            (Digit 2))))))
        (Term
            (Mult
                (Term
                    (Factor
                        (Base
                            (Number
                                (Digit 3)))))
                (Factor
                    (Base
                        (Number
                            (Digit 4))))))
                            
parse_tree_b :: ParseNode
parse_tree_b =
    Expr
        (Term
            (Mult
                (Term
                    (Factor
                        (Base
                            (Add
                                (Expr
                                    (Term
                                        (Factor
                                            (Base
                                                (Number
                                                    (Digit 3))))))
                                (Term
                                    (Factor
                                        (Base
                                            (Number
                                                (Digit 5)))))))))
                (Factor
                    (Base
                        (Sub
                            (Expr
                                (Term
                                    (Factor
                                        (Base
                                            (Number
                                                (Digit 7))))))
                            (Term
                                (Factor
                                    (Base
                                        (Number
                                            (Digit 2))))))))))
                                
parse_tree_c :: ParseNode
parse_tree_c =
    Sub
        (Expr
            (Term
                (Factor
                    (Base
                        (Add
                            (Expr
                                (Term
                                    (Factor
                                        (Base
                                            (Number
                                                (Digit 4))))))
                            (Term
                                (Mult
                                    (Term
                                        (Factor
                                            (Base
                                                (Number
                                                    (Digit 6)))))
                                    (Factor
                                        (Base
                                            (Power
                                                (Base
                                                    (Number
                                                        (Digit 2)))
                                                (Factor
                                                    (Base
                                                        (Number
                                                            (Digit 3))))))))))))))
        (Term
            (Div
                (Term
                    (Factor
                        (Base
                            (Number
                                (Digit 5)))))
                (Factor
                    (Base
                        (Add
                            (Expr
                                (Term
                                    (Factor
                                        (Base
                                            (Number
                                                (Digit 1))))))
                            (Term
                                (Factor
                                    (Base
                                        (Number
                                            (Digit 1))))))))))
                        
                                

parse_tree_d :: ParseNode
parse_tree_d =
    Sub
        (Expr
            (Term
                (Factor
                    (Base
                        (Add
                            (Expr
                                (Term
                                    (Factor
                                        (Base
                                            (Number
                                                (Digit 14))))))
                            (Term
                                (Mult
                                    (Term
                                        (Factor
                                            (Base
                                                (Number
                                                    (Digit 26)))))
                                    (Factor
                                        (Base
                                            (Power
                                                (Base
                                                    (Number
                                                        (Digit 12)))
                                                (Factor
                                                    (Base
                                                        (Number
                                                            (Digit 3))))))))))))))
        (Term
            (Div
                (Term
                    (Factor
                        (Base
                            (Number
                                (Digit 55)))))
                (Factor
                    (Base
                        (Add
                            (Expr
                                (Term
                                    (Factor
                                        (Base
                                            (Number
                                                (Digit 10))))))
                            (Term
                                (Factor
                                    (Base
                                        (Number
                                            (Digit 11))))))))))

expr_eval :: ParseNode -> Integer
expr_eval (Digit n) = n

expr_eval (Number n) = expr_eval n
expr_eval (Base n) = expr_eval n
expr_eval (Factor n) = expr_eval n
expr_eval (Expr n) = expr_eval n
expr_eval (Term n) = expr_eval n

expr_eval (Add e1 e2) = (expr_eval e1) + (expr_eval e2)
expr_eval (Sub e1 e2) = (expr_eval e1) - (expr_eval e2)
expr_eval (Mult e1 e2) = (expr_eval e1) * (expr_eval e2)
expr_eval (Div e1 e2) = (expr_eval e1) `div` (expr_eval e2)
expr_eval (Power e1 e2) = expr_eval e1 ^ expr_eval e2


