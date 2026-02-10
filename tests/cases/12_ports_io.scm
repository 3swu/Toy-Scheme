(port? (current-input-port))
(port? (current-output-port))

(define out (open-output-file "test-artifacts/io_case.scm"))
(write '(1 2 3) out)
(newline out)
(display "(+ 8 9)" out)
(newline out)
(close-output-port out)

(define in (open-input-file "test-artifacts/io_case.scm"))
(read in)
(close-input-port in)
