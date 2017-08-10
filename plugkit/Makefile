ELECTRON_VERSION = $(shell jq '.devDependencies."negatron"' ../../package.json -r)
TOKEN_KEYWORDS=src/token.keys
TOKEN_HASH_TABLE=src/token_hash.h

ifeq ($(OS),Windows_NT)
GYP=..\..\node_modules\.bin\node-gyp
CONFIG = build/binding.sln
else
GYP=../../node_modules/.bin/node-gyp
CONFIG = build/Makefile
ENV = JOBS=4 HOME=~/.electron-gyp
endif

all: $(TOKEN_HASH_TABLE) $(CONFIG)
	$(ENV) $(GYP) build --target=$(ELECTRON_VERSION) \
		--arch=x64 --dist-url=https://atom.io/download/electron

$(CONFIG):
	$(ENV) $(GYP) configure --target=$(ELECTRON_VERSION) \
		--arch=x64 --dist-url=https://atom.io/download/electron

$(TOKEN_HASH_TABLE): $(TOKEN_KEYWORDS)
	gperf $(TOKEN_KEYWORDS) -LANSI-C -G --output-file=$(TOKEN_HASH_TABLE)

clean:
	@$(GYP) clean

fmt:
	@clang-format -i src/*.cpp src/*.hpp \
		include/plugkit/*.h \
		src/wrapper/*.hpp src/wrapper/*.cpp \
		**/*.hpp **/*.cpp

.PHONY: all clean fmt
