#!/bin/sh
(cd dyr_ffi && cargo test && cargo test --release)
(cd dyr_kernel && cargo test && cargo test --release)
(cd dyr_sdk && cargo test && cargo test --release)
#(cd dyr_node && npm i && npm test)
