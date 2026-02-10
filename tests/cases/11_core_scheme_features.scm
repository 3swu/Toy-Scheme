`(1 ,(+ 1 2) ,@(list 4 5))
'(a . b)

(define-syntax when
  (syntax-rules ()
    ((when test body ...)
     (if test (begin body ...) #f))))

(when #t 11 22)
(when #f 1 2)

(define v #(1 2 3))
(vector-length v)
(vector-ref v 1)
(vector-set! v 1 9)
v
(vector->list v)
(list->vector '(7 8))

(char->integer #\a)
(integer->char 98)
(char? #\space)

(eqv? #\a #\a)
(equal? '(1 (2 3)) '(1 (2 3)))
(equal? #(1 2) #(1 2))

(apply + 1 2 (list 3 4))
(map + '(1 2 3) '(4 5 6))

(define sum 0)
(for-each (lambda (a b) (set! sum (+ sum (+ a b)))) '(1 2) '(3 4))
sum

(call/cc (lambda (k) (+ 1 (k 7) 2)))
