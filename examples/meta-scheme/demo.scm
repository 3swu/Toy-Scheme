(load "examples/meta-scheme/meta-interpreter.scm")

(define (demo-run expr)
  (display "meta> ")
  (write expr)
  (newline)
  (display "=> ")
  (write (m-run expr))
  (newline)
  (newline))

(m-reset!)

(demo-run '(define (fact n)
             (if (= n 0)
                 1
                 (* n (fact (- n 1))))))

(demo-run '(fact 6))
(demo-run '(let ((x 7) (y 8)) (+ x y)))
(demo-run '(cond ((> 2 3) 0) ((< 2 3) 99) (else -1)))
(demo-run '(and #t 1 2 3))
(demo-run '(or #f #f 42))
(demo-run '((lambda (x . rest) (list x rest)) 10 20 30 40))
(demo-run '(map (lambda (x) (* x x)) '(1 2 3 4)))
(demo-run '(vector->list (list->vector '(a b c))))
