# Styleguides

## JavaScript

Use [`eslint`](https://eslint.org/) before sending a pull request or CI will fail.

```bash
$ eslint --fix .
```

## Rust

Consider using [`rustfmt`](https://github.com/rust-lang-nursery/rustfmt) for code styling.

```bash
$ cargo +nightly fmt
```

## C/C++

Consider using [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) for code styling.

```bash
$ clang-format -i source.cpp
```
