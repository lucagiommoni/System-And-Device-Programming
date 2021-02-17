# Exercise 3

Implement a C program that

- takes from the command line two integer numbers n1, n2,

- allocates two vectors v1 and v2, of dimensions n1 and n2, respectively,

- fills v1 with n1 random even integer numbers between 10-100,

- fills v2 with n2 random odd integer numbers between 21-101,

- save the content of vectors v1 and v2 in two binary files fv1.b and fv2.b, respectively,

Use command od for verifying the content of files fv1.b, fv2.b.

#### You can use Standard ANSI C
```
// file pointer
FILE *fpw;

// w for write, b for binary
if ((fpw = fopen (filename, "wb")) == NULL) {
  fprinf(stderr," error open %s\n", filename);
  return(1);
}

// write sizeof(buffer) bytes from buffer
fwrite(buffer,sizeof(buffer),1, fpw);
```

#### Or Unix system calls

```
#include <unistd.h>
#include <fcntl.h>

// file descriptor
int fdo;

if ((fdo = open(filename, O_CREAT | O_WRONLY, 0777)) < 0) {
  fprinf(stderr," error open %s\n", filename);
  return 1;
}

write(fdo, buffer, sizeof(buffer));
```
