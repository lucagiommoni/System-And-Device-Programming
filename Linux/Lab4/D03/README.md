# Exercise 2

Implement a **concurrent program** using threads in C that takes a single argument `k` from the command line.

The main thread creates two vectors (`v1` and `v2`) of dimension `k`, a matrix (`mat`) of dimension `k x k`, which are filled with random numbers in the range `[-0.5 ; 0.5]`, and `k` **threads**. Then, it waits the termination of the other threads.

Each thread `i` performs the product of the _i-th_ row vector of `mat` and `v2`, which produces the _i-th_ element of vector `v`.

One of the created threads, the last one terminating its product operation, performs the final operation `result = v1T * v`, and prints the result.

#### This is an example for `k = 5`:

```
v1T = [-0.0613 -0.1184 0.2655 0.2952 -0.3131]

mat =
[ -0.3424   -0.3581    0.1557    0.2577    0.2060
   0.4706   -0.0782   -0.4643    0.2431   -0.4682
   0.4572    0.4157    0.3491   -0.1078   -0.2231
  -0.0146    0.2922    0.4340    0.1555   -0.4538
   0.3003    0.4595    0.1787   -0.3288   -0.4029 ]

v2T = [-0.3235 0.1948 -0.1829 0.4502 -0.4656]

Result: 0.0194
```

Perform the product operation in two steps:

- `v = mat * v2`, which produces a new vector `v`

- `result = v1T * v`
