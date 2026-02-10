; SQL subset engine for Toy-Scheme
; Query form (list of clauses):
;   '((select id name salary)
;     (from "examples/sql-engine/data/employees.csv")
;     (where (and (> salary 120) (< age 40)))
;     (order-by salary desc)
;     (limit 3))

(define (caar x) (car (car x)))
(define (cadr x) (car (cdr x)))
(define (cdar x) (cdr (car x)))
(define (cddr x) (cdr (cdr x)))
(define (caddr x) (car (cdr (cdr x))))

(define (char=? a b) (eqv? a b))
(define (not x) (if x #f #t))
(define (<= a b) (or (< a b) (= a b)))
(define (>= a b) (or (> a b) (= a b)))

(define (reverse-list xs)
  (define (iter rest acc)
    (if (null? rest)
        acc
        (iter (cdr rest) (cons (car rest) acc))))
  (iter xs '()))

(define (whitespace-char? c)
  (let ((n (char->integer c)))
    (or (= n 32) (= n 9) (= n 10) (= n 13))))

(define (trim-left s)
  (let ((n (string-length s)))
    (define (loop i)
      (if (= i n)
          ""
          (if (whitespace-char? (string-ref s i))
              (loop (+ i 1))
              (substring s i n))))
    (loop 0)))

(define (trim-right s)
  (let ((n (string-length s)))
    (if (= n 0)
        ""
        (begin
          (define (iter i)
            (if (< i 0)
                ""
                (if (whitespace-char? (string-ref s i))
                    (iter (- i 1))
                    (substring s 0 (+ i 1)))))
          (iter (- n 1))))))

(define (trim s)
  (trim-right (trim-left s)))

(define (split-csv-line line)
  (let ((n (string-length line)))
    (begin
      (define (iter i start acc)
        (if (= i n)
            (reverse-list (cons (substring line start i) acc))
            (if (= (char->integer (string-ref line i)) 44)
                (iter (+ i 1) (+ i 1) (cons (substring line start i) acc))
                (iter (+ i 1) start acc))))
      (iter 0 0 '()))))

(define (digit-char? c)
  (let ((n (char->integer c)))
    (and (>= n 48) (<= n 57))))

(define (integer-string? s)
  (let ((n (string-length s)))
    (if (= n 0)
        #f
        (let ((start (if (= (char->integer (string-ref s 0)) 45) 1 0)))
          (if (= start n)
              #f
              (begin
                (define (iter i)
                  (if (= i n)
                      #t
                      (if (digit-char? (string-ref s i))
                          (iter (+ i 1))
                          #f)))
                (iter start)))))))

(define (parse-cell s)
  (let ((v (trim s)))
    (if (integer-string? v)
        (string->number v)
        v)))

(define (zip headers values)
  (if (or (null? headers) (null? values))
      '()
      (cons (cons (car headers) (car values))
            (zip (cdr headers) (cdr values)))))

(define (assoc-key key alist)
  (if (null? alist)
      #f
      (if (eq? (caar alist) key)
          (car alist)
          (assoc-key key (cdr alist)))))

(define (row-get row key)
  (let ((p (assoc-key key row)))
    (if p (cdr p) '())))

(define (read-csv path)
  (let ((in (open-input-file path)))
    (let ((header-line (read-line in)))
      (if (eof-object? header-line)
          (begin
            (close-input-port in)
            (list '() '()))
          (let ((headers (map (lambda (s) (string->symbol (trim s)))
                              (split-csv-line header-line))))
            (begin
              (define (iter rows)
                (let ((line (read-line in)))
                  (if (eof-object? line)
                      (begin
                        (close-input-port in)
                        (list headers (reverse-list rows)))
                      (let ((clean (trim line)))
                        (if (= (string-length clean) 0)
                            (iter rows)
                            (let ((cells (map parse-cell (split-csv-line line))))
                              (iter (cons (zip headers cells) rows))))))))
              (iter '())))))))

(define (string<? a b)
  (let ((na (string-length a))
        (nb (string-length b)))
    (begin
      (define (iter i)
        (if (or (= i na) (= i nb))
            (< na nb)
            (let ((ca (char->integer (string-ref a i)))
                  (cb (char->integer (string-ref b i))))
              (if (< ca cb)
                  #t
                  (if (> ca cb)
                      #f
                      (iter (+ i 1)))))))
      (iter 0))))

(define (value=? a b)
  (cond ((and (integer? a) (integer? b)) (= a b))
        ((and (string? a) (string? b)) (eq? a b))
        (else (eq? a b))))

(define (value<? a b)
  (cond ((and (integer? a) (integer? b)) (< a b))
        ((and (string? a) (string? b)) (string<? a b))
        (else #f)))

(define (resolve-value x row)
  (if (symbol? x)
      (row-get row x)
      x))

(define (eval-and args row)
  (if (null? args)
      #t
      (if (eval-predicate (car args) row)
          (eval-and (cdr args) row)
          #f)))

(define (eval-or args row)
  (if (null? args)
      #f
      (if (eval-predicate (car args) row)
          #t
          (eval-or (cdr args) row))))

(define (eval-predicate expr row)
  (if (pair? expr)
      (let ((op (car expr))
            (args (cdr expr)))
        (cond
          ((eq? op 'and) (eval-and args row))
          ((eq? op 'or) (eval-or args row))
          ((eq? op 'not) (if (eval-predicate (car args) row) #f #t))
          ((eq? op '=)
           (value=? (resolve-value (car args) row)
                    (resolve-value (cadr args) row)))
          ((eq? op '<)
           (value<? (resolve-value (car args) row)
                   (resolve-value (cadr args) row)))
          ((eq? op '>)
           (value<? (resolve-value (cadr args) row)
                   (resolve-value (car args) row)))
          ((eq? op '<=)
           (let ((left (resolve-value (car args) row))
                 (right (resolve-value (cadr args) row)))
             (or (value<? left right) (value=? left right))))
          ((eq? op '>=)
           (let ((left (resolve-value (car args) row))
                 (right (resolve-value (cadr args) row)))
             (or (value<? right left) (value=? left right))))
          (else #f)))
      (if expr #t #f)))

(define (filter-list pred xs)
  (if (null? xs)
      '()
      (if (pred (car xs))
          (cons (car xs) (filter-list pred (cdr xs)))
          (filter-list pred (cdr xs)))))

(define (take xs n)
  (if (or (null? xs) (<= n 0))
      '()
      (cons (car xs) (take (cdr xs) (- n 1)))))

(define (insert-by x sorted less?)
  (if (null? sorted)
      (list x)
      (if (less? x (car sorted))
          (cons x sorted)
          (cons (car sorted)
                (insert-by x (cdr sorted) less?)))))

(define (insertion-sort xs less?)
  (if (null? xs)
      '()
      (insert-by (car xs)
                 (insertion-sort (cdr xs) less?)
                 less?)))

(define (find-clause query key)
  (if (null? query)
      #f
      (if (and (pair? (car query)) (eq? (caar query) key))
          (car query)
          (find-clause (cdr query) key))))

(define (query-columns query headers)
  (let ((c (find-clause query 'select)))
    (if (or (not c) (null? (cdr c)))
        headers
        (if (eq? (cadr c) '*)
            headers
            (cdr c)))))

(define (query-from-file query)
  (let ((c (find-clause query 'from)))
    (if (and c (pair? (cdr c)))
        (cadr c)
        "")))

(define (query-where query)
  (let ((c (find-clause query 'where)))
    (if (and c (pair? (cdr c)))
        (cadr c)
        #f)))

(define (query-order query)
  (let ((c (find-clause query 'order-by)))
    (if c c #f)))

(define (query-limit query)
  (let ((c (find-clause query 'limit)))
    (if (and c (pair? (cdr c)) (integer? (cadr c)))
        (cadr c)
        -1)))

(define (project-row row cols)
  (map (lambda (col) (row-get row col)) cols))

(define (value->string v)
  (cond ((integer? v) (number->string v))
        ((boolean? v) (if v "#t" "#f"))
        ((string? v) v)
        ((symbol? v) (symbol->string v))
        (else "?")))

(define (join-with sep parts)
  (if (null? parts)
      ""
      (begin
        (define (iter rest acc)
          (if (null? rest)
              acc
              (iter (cdr rest)
                    (string-append acc sep (car rest)))))
        (iter (cdr parts) (car parts)))))

(define (print-row values)
  (display (join-with " | " (map value->string values)))
  (newline))

(define (print-result result)
  (let ((cols (car result))
        (rows (cadr result)))
    (print-row cols)
    (for-each print-row rows)))

(define (execute-query query)
  (let ((path (query-from-file query))
        (table '())
        (headers '())
        (rows '())
        (where-expr #f)
        (filtered '())
        (order-clause #f)
        (sorted '())
        (cols '()))
    (if (= (string-length path) 0)
        (list '() '())
        (begin
          (set! table (read-csv path))
          (set! headers (car table))
          (set! rows (cadr table))
          (set! where-expr (query-where query))
          (set! filtered
                (if where-expr
                    (filter-list (lambda (row) (eval-predicate where-expr row)) rows)
                    rows))
          (set! order-clause (query-order query))
          (set! sorted
                (if order-clause
                    (let ((order-col (cadr order-clause))
                          (order-dir (if (and (pair? (cddr order-clause))
                                              (eq? (caddr order-clause) 'desc))
                                         'desc
                                         'asc)))
                      (insertion-sort
                       filtered
                       (if (eq? order-dir 'desc)
                           (lambda (a b)
                             (value<? (row-get b order-col)
                                     (row-get a order-col)))
                           (lambda (a b)
                             (value<? (row-get a order-col)
                                     (row-get b order-col))))))
                    filtered))
          (set! cols (query-columns query headers))
          (list cols
                (map (lambda (row) (project-row row cols))
                     (if (> (query-limit query) -1)
                         (take sorted (query-limit query))
                         sorted)))))))

(define (run-query query)
  (let ((result (execute-query query)))
    (print-result result)
    result))
