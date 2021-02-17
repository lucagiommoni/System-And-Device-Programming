# Exercise 01 Lab 13

To compute each term, threads exploit 2 input variable contained into the following struct:

```
typedef struct TRDIN {
	UINT powerHigh, power;
} TRDIN;
```

The following struct, indeed, is used to synchronized *fastest* thread in writing results:

```
typedef struct LAST {
	HANDLE sem;
	CRITICAL_SECTION cs;
	UINT num, max;
} LAST;
```
