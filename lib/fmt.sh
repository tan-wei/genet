#!/bin/sh
(cd genet-abi && cargo +nightly fmt)
(cd genet-kernel && cargo +nightly fmt)
(cd genet-sdk && cargo +nightly fmt)
(cd genet-node && clang-format -i src/*.*)
