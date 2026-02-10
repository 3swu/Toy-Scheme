(load "examples/sql-engine/sql_engine.scm")

(execute-query
 '((select name salary)
   (from "tests/fixtures/sql_employees.csv")
   (where (and (> salary 120) (< age 40)))
   (order-by salary desc)
   (limit 2)))
