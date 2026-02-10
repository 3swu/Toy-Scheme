# Linear Regression Example (Batch Gradient Descent)

This example implements linear regression training with full-batch gradient descent in Scheme and runs it on Toy-Scheme.

Because Toy-Scheme currently only exposes integer arithmetic primitives, the training loop uses fixed-point integers (`SCALE`) and prints results as rational pairs `(numerator . denominator)`.

## Run

```bash
./Toy-Scheme -f examples/ml-linear-regression/linear_regression.scm
```

## What it prints

- training dataset
- centered training dataset (`x - mean(x)`) used by gradient descent
- fitted parameters `(w, b)` as rationals (converted back to the original `y = w*x + b` form)
- training MSE as a rational
- predictions for test `x` values
- fixed-point parameter values (`xSCALE`) for `w` and `b`
