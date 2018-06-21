#!/bin/sh
(cd genet_abi && cargo +nightly fmt)
(cd genet_kernel && cargo +nightly fmt)
(cd genet_sdk && cargo +nightly fmt)
(cd genet_node && clang-format -i src/*.*)
