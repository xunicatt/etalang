# eta-lang(.n)

eta is a lightweight, interpreted language with automatic memory management and a delightfully minimal syntax.
this project is currently in alpha stages.

# dependenices

- [libffi](https://sourceware.org/libffi/)

## install:

### macos

```sh
brew install libffi
```

### linux

```
#Debian/Ubuntu
sudo apt install libffi-dev

#RHEL/Fedora
sudo dnf install libffi-devel

#Arch
sudo pacman -S libffi
```

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

- lib(...): `loads a dynamic library`
- os(...): `get OS platform name, ex: "linux", "darwin"`
- print(...): `prints elements to the console`
- println(...): `same as print() but shift the cursor to new line`
- len(...): `get the length for string and array type`
- type_of(...): `returns the type in string`
- push(...): `pushes an element into an array`
- pop(...): `pop an element from array`
- slice(...): `can be used to slice an array`
- to_int(...): `typecasts to int`
- to_float(...): `typecasts to float`
- read_int(): `reads an integer from stdin`
- read_float(): `reads an float from stdin`
- read_string(): `reads an string from stdin`
- clone(...): `creates an clone of a variable`
