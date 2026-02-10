(define a (lambda x x))
(a 3)
(a 3 4)

(define b (lambda (x y) (+ x y)))
(b 1)
(+ 1 2)
(b 1 2 3)
(+ 2 2)

(define c (lambda x (* 3 x)))
(c 3)
(+ 5 6)
