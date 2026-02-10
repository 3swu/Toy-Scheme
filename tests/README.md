# Test Suite Layout

- `cases/*.scm`: Scheme test inputs.
- `expected/*.txt`: Filtered expected outputs for each case.
- `fixtures/*.scm`: helper files used by `load` tests.
- `repl/*.in`: stdin-driven REPL test inputs.
- `repl-expected/*.txt`: filtered expected outputs for REPL tests.

Run all tests:

```bash
./TEST
```

Intermediate test outputs are written to `test-artifacts/` under this project.
