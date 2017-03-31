DEPLUG_CORE = node_modules/deplug-core
DEPLUG_CORE_SRC = deplug-core/main.js
DEPLUG_CORE_RES = $(wildcard deplug-core/*.htm) $(wildcard deplug-core/*.less)
DEPLUG_CORE_MAIN = $(DEPLUG_CORE)/main.js

ROOLUP_EXTERN = electron,plugkit
ROLLUP = node_modules/.bin/rollup

ELECTRON = node_modules/.bin/electron-deplug
ELECTRON_VERSION = $(shell jq '.devDependencies."electron-deplug"' package.json -r)

PACKAGER = node_modules/.bin/electron-packager

all: build
	$(ELECTRON) .

build: $(DEPLUG_CORE_MAIN)

pack: build
	$(PACKAGER) ./ --download.mirror=https://s3-ap-northeast-1.amazonaws.com/deplug-build-junk/electron/v --asar.unpackDir=node_modules/deplug-helper --electron-version=$(ELECTRON_VERSION) --out=./out --overwrite

$(DEPLUG_CORE_MAIN): $(DEPLUG_CORE) $(DEPLUG_CORE_SRC) $(DEPLUG_CORE_RES)
	$(ROLLUP) $(DEPLUG_CORE_SRC) -e $(ROOLUP_EXTERN) --format cjs --output $(DEPLUG_CORE_MAIN)
	@cp $(DEPLUG_CORE_RES) $(DEPLUG_CORE)

$(DEPLUG_CORE):
	@mkdir $(DEPLUG_CORE)

.PHONY: all run build pack clean fmt
