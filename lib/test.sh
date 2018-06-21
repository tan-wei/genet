#!/bin/sh
(cd genet_abi && cargo test && cargo test --release)
(cd genet_kernel && cargo test && cargo test --release)
(cd genet_sdk && cargo test && cargo test --release)
#(cd genet_node && npm i && npm test)
