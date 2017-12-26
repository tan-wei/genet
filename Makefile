DEPLUG_VER = $(shell node scripts/version-string.js)
DEPLUG_CORE = node_modules/@deplug/core

DEPLUG_CORE_RES = $(wildcard core/asset/*.*)
DEPLUG_CORE_RES_OUT = $(addprefix node_modules/@deplug/,$(DEPLUG_CORE_RES))

DEPLUG_CORE_JS = $(wildcard core/*.js) $(wildcard core/*/*/*.js) $(wildcard core/*/*.js)
DEPLUG_CORE_LIB_JS = $(wildcard core/lib/*.js)
DEPLUG_CORE_MAIN_JS = $(wildcard core/*.main.js)
DEPLUG_CORE_JS_OUT = $(addprefix node_modules/@deplug/,$(DEPLUG_CORE_MAIN_JS))

PLUGKIT_SRC = plugkit
PLUGKIT_DST = node_modules/@deplug/plugkit

PLUGKIT_HEADERS = $(wildcard plugkit/include/plugkit/*.h)
PLUGKIT_JS_FILES = $(wildcard plugkit/js/*.js)

BUILTIN_PACKAGES = package

ELECTRON_VERSION = $(shell node scripts/negatron-version-string.js)
ELECTRON_MIRROR = https://cdn.deplug.net/electron/v
ELECTRON_UNPACK = "{node_modules/@deplug/osx-helper,node_modules/nan,package,deplug-modules/bin}"
ELECTRON_IGNORE = "^(core|plugkit|scripts|snippet|debian|docs|images)"

MOCHA = node_modules/mocha/bin/mocha
APPDMG = node_modules/.bin/appdmg
DOCSIFY = node_modules/.bin/docsify

ifeq ($(OS),Windows_NT)
ELECTRON = node_modules\.bin\negatron.cmd
ESLINT = node_modules\.bin\eslint
WEBPACK = node_modules\.bin\webpack
DPM = node_modules\.bin\dpm
else
ELECTRON = node_modules/.bin/negatron
ESLINT = node_modules/.bin/eslint
WEBPACK = node_modules/.bin/webpack
DPM = node_modules/.bin/dpm
endif

all: build
	$(ELECTRON) --enable-logging .

build: plugkit
	node scripts/build-deplug-core.js

lint:
	$(ESLINT) .

fix:
	$(ESLINT) --fix .

plugkit:
	node scripts/run-as-node.js $(ELECTRON) scripts/generate-version-file.js
	node scripts/build-plugkit.js
	node scripts/build-packages.js

test:
	node scripts/run-as-node.js $(ELECTRON) $(MOCHA) plugkit/test
	node scripts/run-as-node.js $(ELECTRON) deplug-modules/core/test.main.js

dmg:
	npm install appdmg
	$(APPDMG) scripts/appdmg.json out/deplug-darwin-amd64.dmg

deb:
	cp -r debian/. out/.debian
	sed -e "s/{{DEPLUG_VERSION}}/$(DEPLUG_VER)/g" debian/DEBIAN/control > out/.debian/DEBIAN/control
	chmod 755 out/.debian/DEBIAN/postinst
	mkdir -p out/.debian/usr/share/icons/hicolor/256x256/apps
	cp images/deplug.png out/.debian/usr/share/icons/hicolor/256x256/apps
	cp -r out/Deplug-linux-x64/. out/.debian/usr/share/deplug
	mv out/.debian/usr/share/deplug/Deplug out/.debian/usr/share/deplug/deplug
	chrpath -r /usr/share/deplug out/.debian/usr/share/deplug/deplug
	(cd out/.debian && fakeroot dpkg-deb --build . ../deplug-linux-amd64.deb)

rpm:
	mkdir -p out/.rpm
	(cd out/.rpm && mkdir -p SOURCES BUILD RPMS SRPMS)
	sed -e "s/{{DEPLUG_VERSION}}/$(DEPLUG_VER)/g" deplug.rpm.spec > out/.rpm/deplug.rpm.spec
	cp -r debian/usr out/.rpm/BUILD
	mkdir -p out/.rpm/BUILD/usr/share/icons/hicolor/256x256/apps
	cp images/deplug.png out/.rpm/BUILD/usr/share/icons/hicolor/256x256/apps
	cp -r out/Deplug-linux-x64/. out/.rpm/BUILD/usr/share/deplug
	mv out/.rpm/BUILD/usr/share/deplug/Deplug out/.rpm/BUILD/usr/share/deplug/deplug
	chrpath -r /usr/share/deplug out/.rpm/BUILD/usr/share/deplug/deplug
	rpmbuild --define "_topdir $(CURDIR)/out/.rpm" -bb out/.rpm/deplug.rpm.spec
	mv out/.rpm/RPMS/*/*.rpm out/deplug-linux-amd64.rpm

winstaller:
	node scripts/winstaller.js
	mv out/DeplugSetup.exe out/deplug-win-amd64.exe

pack:
	electron-packager ./ --download.mirror=$(ELECTRON_MIRROR) \
							--asar.unpackDir=$(ELECTRON_UNPACK) \
							--icon=images/deplug \
							--ignore=$(ELECTRON_IGNORE) \
							--electron-version=$(ELECTRON_VERSION) \
							--win32metadata.CompanyName=Deplug \
							--win32metadata.FileDescription="Next generation packet analyzer" \
							--win32metadata.OriginalFilename=Deplug.exe \
							--win32metadata.ProductName=Deplug \
							--win32metadata.InternalName=Deplug \
							--out=./out --overwrite

node_modules/@deplug/core/asset/%: core/asset/%
	@mkdir -p node_modules/@deplug/core/asset
	cp -f -p $< $@

$(DEPLUG_CORE_JS_OUT): $(DEPLUG_CORE_JS) $(DEPLUG_CORE)
	$(WEBPACK) $(subst node_modules/@deplug/,,$@) $@

$(DEPLUG_CORE):
	@mkdir -p $(DEPLUG_CORE)

docs:
	mkdir -p out
	cp -r -f -p docs/. out/docs
	@node scripts/generate-docs.js c-parser.js ./snippet $(PLUGKIT_HEADERS) out/docs/diss-api-c.md
	@node scripts/generate-docs.js js-parser.js ./snippet $(PLUGKIT_JS_FILES) out/docs/diss-api-js.md
	@node scripts/generate-docs.js js-parser.js ./snippet $(DEPLUG_CORE_LIB_JS) out/docs/core-api-js.md
	@node scripts/generate-token-docs.js package/token-wellknown/tokens.json out/docs/well-known-tokens.md

docs-serve: docs
	$(DOCSIFY) serve ./out/docs

fmt:
	$(MAKE) fmt -C $(PLUGKIT_SRC)

clean:
	@rm -rf $(DEPLUG_CORE) $(PLUGKIT_DST)

.PHONY: all run build fix lint pack clean fmt plugkit test dmg deb rpm winstaller docs docs-serve
