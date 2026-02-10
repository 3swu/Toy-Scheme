# CSV + SQL Subset Engine (Toy-Scheme)

This example implements a small SQL subset engine in Scheme and runs queries on CSV files.

## Supported SQL subset

The query is represented as a Scheme clause list:

```scheme
'((select col1 col2)
  (from "path/to/file.csv")
  (where (and (> score 80) (= dept "eng")))
  (order-by score desc)
  (limit 10))
```

Supported clauses:
- `select` column list or `*`
- `from` CSV file path
- `where` expression with `and/or/not` and comparisons `= < > <= >=`
- `order-by <column> asc|desc`
- `limit <n>`

Notes:
- CSV parser is intentionally simple: comma-separated fields without quoted commas.
- Integer-looking cells are converted to integers; other cells remain strings.

## Run demo

```bash
./Toy-Scheme -f examples/sql-engine/demo.scm
```

## Files

- `examples/sql-engine/sql_engine.scm`: engine implementation
- `examples/sql-engine/data/employees.csv`: demo dataset
- `examples/sql-engine/demo.scm`: runnable demos
