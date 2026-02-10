(load "tests/fixtures/load_target.scm")
loaded-value
loaded-sum
(let* ((a 1)
       (b (+ a 2))
       (c (+ b 3)))
  c)
(letrec ((even? (lambda (n)
                  (if (= n 0)
                      #t
                      (odd? (- n 1)))))
         (odd? (lambda (n)
                 (if (= n 0)
                     #f
                     (even? (- n 1))))))
  (even? 10))
'abc
'(1 2 3)
