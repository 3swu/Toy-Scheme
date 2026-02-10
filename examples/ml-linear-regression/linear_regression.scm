; Linear Regression in Toy-Scheme - Full Batch Gradient Descent Version
; Model: y = w*x + b
;
; Since Toy-Scheme currently only has integer arithmetic primitives, this
; program uses fixed-point integers for training:
;   real_value ~= scaled_value / SCALE

(define (cadr x) (car (cdr x)))
(define (caddr x) (car (cdr (cdr x))))
(define (cadddr x) (car (cdr (cdr (cdr x)))))

(define (abs-int x)
  (if (< x 0) (- 0 x) x))

(define (gcd-int a b)
  (if (= b 0)
      (abs-int a)
      (gcd-int b (remainder a b))))

(define (make-rat n d)
  (if (= d 0)
      '(error divide-by-zero)
      (let ((sign (if (< d 0) -1 1)))
        (let ((nn (* sign n))
              (dd (abs-int d)))
          (let ((g (gcd-int nn dd)))
            (cons (quotient nn g)
                  (quotient dd g)))))))

(define (scaled->rat v scale)
  (make-rat v scale))

(define (div-round n d)
  (if (< n 0)
      (- 0 (quotient (+ (- 0 n) (quotient d 2)) d))
      (quotient (+ n (quotient d 2)) d)))

(define (len xs)
  (if (null? xs)
      0
      (+ 1 (len (cdr xs)))))

(define (sum-x data)
  (if (null? data)
      0
      (+ (car (car data)) (sum-x (cdr data)))))

(define (center-data data x-mean)
  (if (null? data)
      '()
      (let ((p (car data)))
        (cons (cons (- (car p) x-mean) (cdr p))
              (center-data (cdr data) x-mean)))))

; Accumulate:
;   sum_err    = sum(yhat - y)
;   sum_err_x  = sum((yhat - y) * x)
;   sum_sq_err = sum((yhat - y)^2)
(define (grad-stats-loop data w b scale sum-err sum-err-x sum-sq-err)
  (if (null? data)
      (list sum-err sum-err-x sum-sq-err)
      (let ((p (car data)))
        (let ((x (car p))
              (y (cdr p)))
          (let ((yhat-scaled (+ (* w x) b))
                (y-scaled (* y scale)))
            (let ((err (- yhat-scaled y-scaled)))
              (grad-stats-loop (cdr data)
                               w
                               b
                               scale
                               (+ sum-err err)
                               (+ sum-err-x (* err x))
                               (+ sum-sq-err (* err err)))))))))

(define (grad-stats data w b scale)
  (grad-stats-loop data w b scale 0 0 0))

(define (predict-scaled model x)
  (+ (* (car model) x) (cadr model)))

(define (predict-rat model x scale)
  (scaled->rat (predict-scaled model x) scale))

(define (predict-many-loop xs model scale)
  (if (null? xs)
      '()
      (cons (list (car xs) (predict-rat model (car xs) scale))
            (predict-many-loop (cdr xs) model scale))))

(define (predict-many model xs scale)
  (predict-many-loop xs model scale))

(define (print-train-log epoch w b mse-scaled scale)
  (display "[epoch ")
  (write epoch)
  (display "] ")
  (display "w=")
  (write (scaled->rat w scale))
  (display ", b=")
  (write (scaled->rat b scale))
  (display ", mse=")
  (write (make-rat mse-scaled (* scale scale)))
  (newline))

(define (train-gd-loop data
                       n
                       scale
                       lr-num
                       lr-den
                       epochs
                       print-every
                       epoch
                       w
                       b)
  (let ((s (grad-stats data w b scale)))
    (let ((sum-err (car s))
          (sum-err-x (cadr s))
          (sum-sq-err (caddr s)))
      (let ((mse-scaled (quotient sum-sq-err n)))
        (if (= (remainder epoch print-every) 0)
            (print-train-log epoch w b mse-scaled scale)
            'skip)
        (if (= epoch epochs)
            (list (list w b) mse-scaled)
            (let ((den (* lr-den n))
                  (dw-num (* (* 2 lr-num) sum-err-x))
                  (db-num (* (* 2 lr-num) sum-err)))
                (let ((dw (div-round dw-num den))
                      (db (div-round db-num den)))
              (train-gd-loop data
                             n
                             scale
                             lr-num
                             lr-den
                             epochs
                             print-every
                             (+ epoch 1)
                             (- w dw)
                             (- b db)))))))))

(define (fit-linear-regression-gd data scale lr-num lr-den epochs print-every)
  (let ((n (len data)))
    (if (= n 0)
        '(error empty-dataset)
        (train-gd-loop data
                       n
                       scale
                       lr-num
                       lr-den
                       epochs
                       print-every
                       0
                       0
                       0))))

; --------------------------------------------------------------------
; Demo dataset:
; y = 3/2 * x + 1
; points: (2,4), (4,7), (6,10), (8,13)
; --------------------------------------------------------------------
(define train-data
  '((2 . 4)
    (4 . 7)
    (6 . 10)
    (8 . 13)))

; Fixed-point scale and GD hyperparameters
(define SCALE 1000000)
(define LEARNING-RATE-NUM 1)    ; alpha = 1/20000
(define LEARNING-RATE-DEN 20000)
(define EPOCHS 80000)
(define PRINT-EVERY 5000)

(display "train-data: ")
(write train-data)
(newline)

(display "config: ")
(write (list (cons 'scale SCALE)
             (cons 'lr (make-rat LEARNING-RATE-NUM LEARNING-RATE-DEN))
             (cons 'epochs EPOCHS)))
(newline)
(newline)

(define X-MEAN (quotient (sum-x train-data) (len train-data)))
(define centered-train-data (center-data train-data X-MEAN))

(display "x-mean: ")
(write X-MEAN)
(newline)
(display "centered-train-data: ")
(write centered-train-data)
(newline)
(newline)

(define fit-result
  (fit-linear-regression-gd centered-train-data
                            SCALE
                            LEARNING-RATE-NUM
                            LEARNING-RATE-DEN
                            EPOCHS
                            PRINT-EVERY))

(define centered-model (car fit-result))
(define centered-w (car centered-model))
(define centered-b (cadr centered-model))
(define original-b (- centered-b (* centered-w X-MEAN)))
(define model (list centered-w original-b))
(define final-mse-scaled (cadr fit-result))

(define test-xs '(1 3 5 10))
(define test-preds (predict-many model test-xs SCALE))

(newline)
(display "final-model(w,b): ")
(write (list (scaled->rat (car model) SCALE)
             (scaled->rat (cadr model) SCALE)))
(newline)

(display "final-mse: ")
(write (make-rat final-mse-scaled (* SCALE SCALE)))
(newline)

(display "predictions(x, yhat): ")
(write test-preds)
(newline)

(display "w xSCALE: ")
(write (car model))
(newline)

(display "b xSCALE: ")
(write (cadr model))
(newline)
