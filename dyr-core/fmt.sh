#!/bin/sh
(cd dyr_ffi && cargo +nightly fmt)
(cd dyr_kernel && cargo +nightly fmt)
(cd dyr_sdk && cargo +nightly fmt)
(cd dyr_node && clang-format -i src/*.*)
