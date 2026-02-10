(load "examples/sql-engine/sql_engine.scm")

(display "-- Query 1: high paid engineers --")
(newline)
(run-query
 '((select id name salary)
   (from "examples/sql-engine/data/employees.csv")
   (where (and (= dept "eng") (>= salary 150)))
   (order-by salary desc)
   (limit 3)))
(newline)

(display "-- Query 2: younger staff sorted by age --")
(newline)
(run-query
 '((select name dept age)
   (from "examples/sql-engine/data/employees.csv")
   (where (< age 35))
   (order-by age asc)
   (limit 4)))
(newline)

(display "-- Query 3: select all columns --")
(newline)
(run-query
 '((select *)
   (from "examples/sql-engine/data/employees.csv")
   (where (> salary 100))
   (order-by id asc)
   (limit 5)))
