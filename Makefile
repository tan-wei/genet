all:
	npm run watch

build:
	npm run build

test:
	npm test

fix:
	eslint --fix .

fmt:
	-clang-format -i plugkit/**/*.{cpp,hpp,h} plugkit/src/wrapper/*.{cpp,hpp} package/**/*.{cpp,hpp}

gperf:
	gperf -LANSI-C -G plugkit/src/token.keys --output-file=plugkit/src/token_hash.h
	gperf -t -LANSI-C -G plugkit/src/function.keys --output-file=plugkit/src/function_hash.h

.PHONY: all build fix fmt gperf
