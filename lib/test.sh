#!/bin/sh
(cd genet-abi && cargo test && cargo test --release)
(cd genet-kernel && cargo test && cargo test --release)
(cd genet-sdk && cargo test && cargo test --release)
#(cd genet-node && npm i && npm test)
