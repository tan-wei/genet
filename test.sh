#!/bin/sh
(cd genet-abi && cargo test)
(cd genet-kernel && cargo test)
(cd genet-sdk && cargo test)