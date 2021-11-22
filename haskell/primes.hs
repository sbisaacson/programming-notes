import Data.Array.ST
import Control.Monad.ST
import Control.Monad

-- It is very challenging to get code with arrays to typecheck.

primes :: Int -> [Int]
primes bound = [i | (i, True) <- runST sieve]
  where sieve = do arr <- newArray (2, bound) True :: ST s (STUArray s Int Bool)
                   forM_ [2..bound] $ \p ->
                     (do v <- readArray arr p
                         if v then
                           forM_ [p * p, p * (p + 1) .. bound] $ \i -> writeArray arr i False
                         else
                           return ())
                   getAssocs arr

main :: IO ()
main = print $ (primes 200000) !! 10000
