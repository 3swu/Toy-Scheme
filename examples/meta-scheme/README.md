# Meta Scheme Interpreter Example

This example implements a small Scheme interpreter in Scheme, running on Toy-Scheme.

## Run

```bash
./Toy-Scheme -f examples/meta-scheme/demo.scm
```

## Files

- `meta-interpreter.scm`: the interpreter implementation.
- `demo.scm`: demonstration program that loads and uses the interpreter.

## Supported in the meta interpreter

- forms: `quote`, `if`, `lambda`, `begin`, `define`, `set!`, `cond`, `let`, `and`, `or`
- application of host primitives and meta-level closures
- fixed args, variadic args, and dotted parameter lists
