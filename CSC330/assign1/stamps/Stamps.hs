-- Code provided to students in UVic CSC 330 Summer 2026 
-- 
-- Author: Mike Zastre
--
-- The copyright for this code is held by Mike Zastre. It is
-- not to be uploaded or otherwise accessed by any online service
-- or online-facilitated tool (such as Copilot or ChatGPT, although
-- this list is not exhaustive).


import System.IO
import System.Environment (getArgs)
import Data.List

type ProblemTuple = (Integer, [Integer])


-- Top level function: Because Haskell constrains the mixture of
-- of functions having I/O with functions having no I/O, we need
-- to use an IO monad (the "IO ()" syntax).  There should be no
-- need to modify this code unless you _really_ know what you are
-- doing in Haskell.  This has been written for you in order that
-- you may completely ignore I/O issues.
--
-- The first parameter if `stamps` is the name of the input file;
-- the second is the name of the output file.
--
-- THERE SHOULD BE NO NEED TO MODIFY THE CODE UP TO THE POINT
-- WITH THE COMMENT "-- STUDENT WORK BEGINS HERE".
--

main :: IO ()
main = do
    args <- getArgs
    case args of
        (inFile : outFile : _) -> stamps inFile outFile
        _ -> putStrLn "Usage: <input file> <output file>"

stamps :: String -> String -> IO ()
stamps inName outName = do
    inFile <- openFile inName ReadMode
    outFile <- openFile outName WriteMode
    lineData <- hGetContents inFile
    let rawLines = lines lineData
    let problemList = parseData rawLines
    let answers = process problemList
    outputResults outFile answers
    hClose inFile
    hClose outFile

parseData :: [String] -> [ProblemTuple]
parseData [] = []
parseData (a:[]) 
    | a == "0"  = []
    | otherwise = [(-1, [])]
parseData (a:b:as) = (width, stamps) : parseData as
    where
        width = read a :: Integer
        stamps = string2ints (words b)

string2ints :: [String] -> [Integer]
string2ints [] = []
string2ints (a:as) = (read a :: Integer) : string2ints as

ints2string :: [Integer] -> String
ints2string [] = ""
ints2string (a:[]) = show a
ints2string (a:as) = show a ++ " " ++ ints2string as

outputResults :: Handle -> [String] -> IO ()
outputResults _ [] = return ()
outputResults outFile (a:as) = do
    hPutStrLn outFile a
    outputResults outFile as



--
-- STUDENT WORK BEGINS HERE
--
-- YOU ARE FORBIDDEN TO USE EITHER "do" OR "let" IN YOUR
-- SOLUTION. THE APPEARANCE OF EITHER OF THESE KEYWORDS
-- IN YOUR CODE MAY RESULT IN A FAILING GRADE FOR THIS PART
-- OF THE ASSIGNMENT.
--

process :: [ProblemTuple] -> [String]
process [] = []
process (x:xs) = singleProblem x : process xs

singleProblem :: ProblemTuple -> String
singleProblem (maxStamps, stamps) =
    "max coverage = " ++ show(coverage maxStamps (tail stamps)) ++ " : " ++ ints2string (tail stamps)

coverage :: Integer -> [Integer] -> Integer
coverage maxStamps denominations = coverageFrom 1 maxStamps denominations


coverageFrom :: Integer -> Integer -> [Integer] -> Integer
coverageFrom amount maxStamps denominations
    | checkValue amount maxStamps denominations = coverageFrom (amount + 1) maxStamps denominations
    | otherwise = amount - 1



-- Amount (Start 0 count up), stamps left, [stamp denominations] returns bool
checkValue :: Integer -> Integer -> [Integer] -> Bool
checkValue 0 _ _ = True  -- Can always make 0
checkValue _ 0 _ = False -- Cant make anything with no stamps left
checkValue amount stampsLeft denominations =
    tryStamps amount stampsLeft denominations denominations

--Amount, stamps left, denominations, denominations again to remove first element return Bool
tryStamps :: Integer -> Integer -> [Integer] -> [Integer]-> Bool
tryStamps _ _ _ [] = False
tryStamps amount stampsLeft denominations (x:xs)
    | amount - x < 0 = tryStamps amount stampsLeft denominations xs
    
    -- Checks stamps incrementally to find largest x that fits in the ammount
    
    | checkValue (amount - x) (stampsLeft - 1) denominations = True
    
    -- continue checking coverage with smaller amount and one less stamp
    
    | otherwise = tryStamps amount stampsLeft denominations xs
    
    -- Continue checking next x if checkValue comes back false with current x


--
-- Here are two problem tuples to help you get started
--


pt0 :: [ProblemTuple]
pt0 = [(5, [2, 1, 4])]

pt1 :: [ProblemTuple]
pt1 = [(5, [4, 1, 4, 12, 21]),
       (10, [5, 1, 7, 16, 31, 88]),
       (6, [4, 1, 5, 7, 8])
      ]



