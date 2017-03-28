#!/bin/sh
(cd lib/genet-abi && cargo test)
(cd lib/genet-kernel && cargo test)
(cd lib/genet-sdk && cargo test)