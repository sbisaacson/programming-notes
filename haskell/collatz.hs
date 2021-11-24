-- This is ia little messy because we don't memoize everything.  We
-- also require 64-bit Ints. It's about 10x slower than a C solution.
-- (Presumably we could write something with STArray that would be faster.)
import Data.Array
import Data.List (foldl')

next :: Int -> Int
next i =
  if i `mod` 2 == 0
    then i `div` 2
    else 3 * i + 1

collatz :: Int -> Array Int Int
collatz n = a
  where
    a = array (0, n) $ map f [0 .. n]
    f i
      | i <= 1 = (i, 1)
      | otherwise = (i, (1 +) $ g $ next i)
    g i
      | i <= n = a ! i
      | otherwise = (1 +) $ g $ next i

main :: IO ()
main = print $ foldl' max 1 $ collatz 1000000
