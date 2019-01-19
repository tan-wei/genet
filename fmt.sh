#!/bin/sh

find . -name Cargo.toml|while read fname; do
  DIR=$(dirname "$fname")
  (cd $DIR; cargo +nightly fmt; cargo tomlfmt)
done

(cd genet-node && clang-format -i src/*.*)
