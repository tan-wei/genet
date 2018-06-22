#!/bin/sh
(cd genet_abi && cargo test && cargo test --release)
(cd genet_kernel && cargo test && cargo test --release)
(cd genet-sdk && cargo test && cargo test --release)
#(cd genet-node && npm i && npm test)
