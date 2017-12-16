ELECTRON_VERSION = $(shell node ../../scripts/negatron-version-string.js)
TOKEN_KEYWORDS=src/token.keys
TOKEN_HASH_TABLE=src/token_hash.h

JS_FILES = $(wildcard js/*.js)
JS_EMBEDDED_HEADER = src/embedded_files.hpp

ifeq ($(OS),Windows_NT)
GYP=..\..\node_modules\.bin\node-gyp
GPERF=..\..\node_modules\.bin\gperf
CONFIG = build/binding.sln
else
GYP=../../node_modules/.bin/node-gyp
GPERF=gperf -LANSI-C
CONFIG = build/Makefile
ENV = JOBS=4 HOME=~/.electron-gyp
endif

ifeq ($(NODE_ENV),development)
GYP += --debug
endif

all: $(TOKEN_HASH_TABLE) $(JS_EMBEDDED_HEADER)

$(TOKEN_HASH_TABLE): $(TOKEN_KEYWORDS)
	$(GPERF) $(TOKEN_KEYWORDS) -G --output-file=$(TOKEN_HASH_TABLE)

$(JS_EMBEDDED_HEADER): $(JS_FILES)
	node ../../../scripts/text-to-cpp.js $(JS_FILES) $(JS_EMBEDDED_HEADER)

clean:
	@$(GYP) clean

fmt:
	@clang-format -i src/*.cpp src/*.hpp \
		include/plugkit/*.h \
		src/wrapper/*.hpp src/wrapper/*.cpp \
		**/*.hpp **/*.cpp

.PHONY: all clean fmt
