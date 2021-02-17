# Exercise 1

- Write a C program using Pthreads to sort the content of a binary file including a sequence of random integer numbers, passed as an argument of the command line.

- Map the file as a vector in memory.

- Implement a threaded quicksort program where the recursive calls to quicksort are replaced by threads activations, i.e. sorting is done, in parallel, in different regions of the file. If the difference between the right and left indexes is less than a value size, given as an argument of the command line, sorting is performed by the standard quicksort algorithm.

#### This is a sequential recursive implementation of the quicksort algorithm.

```
void quicksort (int v[], int left, int right) {
  int i, j, x, tmp;
  if (left >= right) return;
  x = v[left];
  i = left - 1;
  j = right + 1;
    while (i < j) {
    while (v[--j] > x);
    while (v[++i] < x);
    if (i < j)
    swap (i,j);
  }
  quicksort (v, left, j);
  quicksort (v, j + 1, right);
}

void swap(int i, int j){
  int tmp;
  tmp = v[i];
  v[i] = v[j];
  v[j] = tmp;
}
```
