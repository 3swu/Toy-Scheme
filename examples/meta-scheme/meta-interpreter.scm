; A small Scheme-in-Scheme interpreter running on Toy-Scheme.
; Supported forms:
;   quote, if, lambda, begin, define, set!, cond, let, and, or, application

(define (cadr x) (car (cdr x)))
(define (caddr x) (car (cdr (cdr x))))
(define (cadddr x) (car (cdr (cdr (cdr x)))))
(define (caadr x) (car (car (cdr x))))
(define (cdadr x) (cdr (car (cdr x))))
(define (cddr x) (cdr (cdr x)))
(define (cdddr x) (cdr (cdr (cdr x))))

(define (m-tagged? exp tag)
  (and (pair? exp) (eq? (car exp) tag)))

(define (m-self-evaluating? exp)
  (or (integer? exp)
      (boolean? exp)
      (string? exp)
      (char? exp)
      (vector? exp)))

(define (m-variable? exp)
  (symbol? exp))

(define (m-quoted? exp)
  (m-tagged? exp 'quote))

(define (m-assignment? exp)
  (m-tagged? exp 'set!))

(define (m-definition? exp)
  (m-tagged? exp 'define))

(define (m-if? exp)
  (m-tagged? exp 'if))

(define (m-lambda? exp)
  (m-tagged? exp 'lambda))

(define (m-begin? exp)
  (m-tagged? exp 'begin))

(define (m-cond? exp)
  (m-tagged? exp 'cond))

(define (m-let? exp)
  (m-tagged? exp 'let))

(define (m-and? exp)
  (m-tagged? exp 'and))

(define (m-or? exp)
  (m-tagged? exp 'or))

(define (m-application? exp)
  (pair? exp))

(define (m-text-of-quotation exp)
  (cadr exp))

(define (m-if-predicate exp)
  (cadr exp))

(define (m-if-consequent exp)
  (caddr exp))

(define (m-if-alternative exp)
  (if (null? (cdddr exp)) #f (cadddr exp)))

(define (m-lambda-parameters exp)
  (cadr exp))

(define (m-lambda-body exp)
  (cddr exp))

(define (m-begin-actions exp)
  (cdr exp))

(define (m-first-exp seq)
  (car seq))

(define (m-rest-exps seq)
  (cdr seq))

(define (m-last-exp? seq)
  (null? (cdr seq)))

(define (m-assignment-variable exp)
  (cadr exp))

(define (m-assignment-value exp)
  (caddr exp))

(define (m-definition-variable exp)
  (if (symbol? (cadr exp))
      (cadr exp)
      (caadr exp)))

(define (m-definition-value exp)
  (if (symbol? (cadr exp))
      (caddr exp)
      (cons 'lambda (cons (cdadr exp) (cddr exp)))))

(define (m-cond-clauses exp)
  (cdr exp))

(define (m-cond-else-clause? clause)
  (eq? (car clause) 'else))

(define (m-sequence->exp seq)
  (cond ((null? seq) #f)
        ((m-last-exp? seq) (m-first-exp seq))
        (else (cons 'begin seq))))

(define (m-expand-clauses clauses)
  (if (null? clauses)
      #f
      (let ((first (car clauses))
            (rest (cdr clauses)))
        (if (m-cond-else-clause? first)
            (if (null? rest)
                (m-sequence->exp (cdr first))
                '(quote (meta-error cond-else-not-last)))
            (list 'if
                  (car first)
                  (m-sequence->exp (cdr first))
                  (m-expand-clauses rest))))))

(define (m-cond->if exp)
  (m-expand-clauses (m-cond-clauses exp)))

(define (m-let-bindings exp)
  (cadr exp))

(define (m-let-body exp)
  (cddr exp))

(define (m-binding-param binding)
  (car binding))

(define (m-binding-arg binding)
  (cadr binding))

(define (m-let-params bindings)
  (if (null? bindings)
      '()
      (cons (m-binding-param (car bindings))
            (m-let-params (cdr bindings)))))

(define (m-let-args bindings)
  (if (null? bindings)
      '()
      (cons (m-binding-arg (car bindings))
            (m-let-args (cdr bindings)))))

(define (m-let->application exp)
  (cons (cons 'lambda
              (cons (m-let-params (m-let-bindings exp))
                    (m-let-body exp)))
        (m-let-args (m-let-bindings exp))))

(define (m-operator exp)
  (car exp))

(define (m-operands exp)
  (cdr exp))

(define (m-no-operands? ops)
  (null? ops))

(define (m-first-operand ops)
  (car ops))

(define (m-rest-operands ops)
  (cdr ops))

(define (m-true? value)
  (if value #t #f))

(define (m-false? value)
  (if value #f #t))

(define (m-find-binding frame var)
  (if (null? frame)
      #f
      (if (eq? (car (car frame)) var)
          (car frame)
          (m-find-binding (cdr frame) var))))

(define (m-lookup-variable-value var env)
  (if (null? env)
      (list 'meta-error 'unbound-variable var)
      (let ((binding (m-find-binding (car env) var)))
        (if binding
            (cdr binding)
            (m-lookup-variable-value var (cdr env))))))

(define (m-set-variable-value! var val env)
  (if (null? env)
      (list 'meta-error 'unbound-variable var)
      (let ((binding (m-find-binding (car env) var)))
        (if binding
            (begin (set-cdr! binding val) 'ok)
            (m-set-variable-value! var val (cdr env))))))

(define (m-define-variable! var val env)
  (let ((frame (car env)))
    (let ((binding (m-find-binding frame var)))
      (if binding
          (set-cdr! binding val)
          (set-car! env (cons (cons var val) frame)))))
  'ok)

(define (m-make-closure params body env)
  (list 'closure params body env))

(define (m-closure? proc)
  (and (pair? proc) (eq? (car proc) 'closure)))

(define (m-closure-parameters proc)
  (cadr proc))

(define (m-closure-body proc)
  (caddr proc))

(define (m-closure-environment proc)
  (cadddr proc))

(define (m-bind-parameters params args)
  (cond ((symbol? params)
         (list (cons params args)))
        ((null? params)
         (if (null? args)
             '()
             '(meta-error too-many-arguments)))
        ((pair? params)
         (if (null? args)
             '(meta-error too-few-arguments)
             (cons (cons (car params) (car args))
                   (m-bind-parameters (cdr params) (cdr args)))))
        (else '(meta-error invalid-parameter-list))))

(define (m-extend-environment params args base-env)
  (cons (m-bind-parameters params args) base-env))

(define (m-eval-list exps env)
  (if (m-no-operands? exps)
      '()
      (cons (m-eval (m-first-operand exps) env)
            (m-eval-list (m-rest-operands exps) env))))

(define (m-eval-sequence seq env)
  (if (m-last-exp? seq)
      (m-eval (m-first-exp seq) env)
      (begin
        (m-eval (m-first-exp seq) env)
        (m-eval-sequence (m-rest-exps seq) env))))

(define (m-eval-and tests env)
  (if (null? tests)
      #t
      (if (m-last-exp? tests)
          (m-eval (car tests) env)
          (let ((value (m-eval (car tests) env)))
            (if (m-false? value)
                #f
                (m-eval-and (cdr tests) env))))))

(define (m-eval-or tests env)
  (if (null? tests)
      #f
      (if (m-last-exp? tests)
          (m-eval (car tests) env)
          (let ((value (m-eval (car tests) env)))
            (if (m-true? value)
                value
                (m-eval-or (cdr tests) env))))))

(define (m-apply proc args)
  (cond ((m-closure? proc)
         (m-eval-sequence
          (m-closure-body proc)
          (m-extend-environment
           (m-closure-parameters proc)
           args
           (m-closure-environment proc))))
        ((procedure? proc)
         (apply proc args))
        (else
         (list 'meta-error 'unknown-procedure proc))))

(define (m-eval exp env)
  (cond ((m-self-evaluating? exp) exp)
        ((m-variable? exp) (m-lookup-variable-value exp env))
        ((m-quoted? exp) (m-text-of-quotation exp))
        ((m-assignment? exp)
         (m-set-variable-value!
          (m-assignment-variable exp)
          (m-eval (m-assignment-value exp) env)
          env))
        ((m-definition? exp)
         (m-define-variable!
          (m-definition-variable exp)
          (m-eval (m-definition-value exp) env)
          env))
        ((m-if? exp)
         (if (m-true? (m-eval (m-if-predicate exp) env))
             (m-eval (m-if-consequent exp) env)
             (m-eval (m-if-alternative exp) env)))
        ((m-lambda? exp)
         (m-make-closure
          (m-lambda-parameters exp)
          (m-lambda-body exp)
          env))
        ((m-begin? exp)
         (m-eval-sequence (m-begin-actions exp) env))
        ((m-cond? exp)
         (m-eval (m-cond->if exp) env))
        ((m-let? exp)
         (m-eval (m-let->application exp) env))
        ((m-and? exp)
         (m-eval-and (cdr exp) env))
        ((m-or? exp)
         (m-eval-or (cdr exp) env))
        ((m-application? exp)
         (m-apply
          (m-eval (m-operator exp) env)
          (m-eval-list (m-operands exp) env)))
        (else (list 'meta-error 'unknown-expression exp))))

(define (m-host-not x)
  (if x #f #t))

(define (m-proper-list? items)
  (if (null? items)
      #t
      (if (pair? items)
          (m-proper-list? (cdr items))
          #f)))

(define (m-prim-apply proc arglist)
  (if (m-proper-list? arglist)
      (m-apply proc arglist)
      '(meta-error apply-arg-must-be-list)))

(define (m-prim-map proc items)
  (if (null? items)
      '()
      (if (pair? items)
          (cons (m-apply proc (list (car items)))
                (m-prim-map proc (cdr items)))
          '(meta-error map-arg-must-be-list))))

(define (m-prim-for-each proc items)
  (if (null? items)
      'ok
      (if (pair? items)
          (begin
            (m-apply proc (list (car items)))
            (m-prim-for-each proc (cdr items)))
          '(meta-error for-each-arg-must-be-list))))

(define (m-primitive-bindings)
  (list
   (cons '+ +)
   (cons '- -)
   (cons '* *)
   (cons '/ /)
   (cons '= =)
   (cons '< <)
   (cons '> >)
   (cons 'cons cons)
   (cons 'car car)
   (cons 'cdr cdr)
   (cons 'set-car! set-car!)
   (cons 'set-cdr! set-cdr!)
   (cons 'list list)
   (cons 'null? null?)
   (cons 'pair? pair?)
   (cons 'symbol? symbol?)
   (cons 'integer? integer?)
   (cons 'number? number?)
   (cons 'boolean? boolean?)
   (cons 'string? string?)
   (cons 'char? char?)
   (cons 'vector? vector?)
   (cons 'vector vector)
   (cons 'vector-length vector-length)
   (cons 'vector-ref vector-ref)
   (cons 'vector-set! vector-set!)
   (cons 'vector->list vector->list)
   (cons 'list->vector list->vector)
   (cons 'eq? eq?)
   (cons 'eqv? eqv?)
   (cons 'equal? equal?)
   (cons 'not m-host-not)
   (cons 'apply m-prim-apply)
   (cons 'map m-prim-map)
   (cons 'for-each m-prim-for-each)
   (cons 'display display)
   (cons 'newline newline)
   (cons 'write write)))

(define (m-make-global-environment)
  (list (m-primitive-bindings)))

(define m-global-env (m-make-global-environment))

(define (m-run expr)
  (m-eval expr m-global-env))

(define (m-reset!)
  (set! m-global-env (m-make-global-environment))
  'ok)
