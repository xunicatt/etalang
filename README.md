# eta-lang(.n)

an interpreted toy language built for fun!
this project is currently in alpha stages.

# syntax

checkout `example/` directory for language syntax

# build

```bash
make #to build and install
make uninstall #to uninstall

eta #for repl
eta <filename>.n #for file input
```

## inbuilt functions

- print(...): `prints elements to the console`
- println(...): `same as print() but shift the cursor to new line`
- len(...): `get the length for string and array type`
- type(...): `returns the type in string format, ex: "int", "float", "string", "bool", "function"`
- push(...): `pushes an element into an array`
- pop(...): `pop an element from array`
- slice(...): `can be used to copy a array by value or to slice an array, ex: slice(arr, 0, 2)`
- int(...): `typecasts to int`
- float(...): `typecasts to float`
- readint(): `reads an integer from stdin`
- readfloat(): `reads an float from stdin`
- readstring(): `reads an string from stdin`
