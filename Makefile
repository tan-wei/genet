ELECTRON_VERSION = $(shell jq '.devDependencies."electron-deplug"' package.json -r)
GYPCONFING = binding.gyp

ifeq ($(OS),Windows_NT)

CONFIG = build/binding.sln
all: $(CONFIG)
	node-gyp build --target=$(ELECTRON_VERSION) \
		--arch=x64 --dist-url=https://atom.io/download/electron

$(CONFIG): $(GYPCONFING)
	node-gyp configure --target=$(ELECTRON_VERSION) \
		--arch=x64 --dist-url=https://atom.io/download/electron

else

CONFIG = build/Makefile
all: $(CONFIG)
	JOBS=4 HOME=~/.electron-gyp node-gyp build \
		--target=$(ELECTRON_VERSION) --arch=x64 \
		--dist-url=https://atom.io/download/electron

$(CONFIG): $(GYPCONFING)
	HOME=~/.electron-gyp node-gyp configure \
		--target=$(ELECTRON_VERSION) --arch=x64 \
		--dist-url=https://atom.io/download/electron

endif

$(GYPCONFING): $(GYPCONFING).plugkit
	cp $(GYPCONFING).plugkit $(GYPCONFING)

clean:
	@node-gyp clean
	@rm $(GYPCONFING)

fmt:
	@clang-format -i src/*.cpp src/*.hpp \
		include/plugkit/*.hpp src/private/*.hpp \
		src/wrapper/*.hpp src/wrapper/*.cpp \
		**/*.hpp **/*.cpp

.PHONY: all clean fmt
