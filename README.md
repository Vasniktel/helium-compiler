# Helium programming language
*take a deep breath*

---

This repo is a home for the Helium programming language.
It is still in its early development.

The language features:
* static typing
* AOT compilation to bytecode
* expression-based syntax
* TODO

## Examples

### Disclaimer
*Everything in this section is still only a concept and is
subject to change in the future*

* Hello world
```kt
println("Hello world")
```

* Range from 1 to 10
```kt
for (i, val : indexed(1 .. 10)) println(val)
```

* Fibonacci
```kt
fun fib(n: Int): Int {
    if (n <= 1) 1 else fib(n - 1) + fib(n - 2)
}
```

* Variables
```kt
var f = 4
var f: Int = 4
var k, p: Int = (3, 4)
```

TODO
