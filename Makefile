DEPLUG_VER = $(shell jq -r '.version' package.json)
DEPLUG_CORE = node_modules/deplug-core node_modules/deplug-core/theme

DEPLUG_CORE_RES = $(wildcard deplug-core/*.htm) $(wildcard deplug-core/theme/*.*)
DEPLUG_CORE_RES_OUT = $(addprefix node_modules/,$(DEPLUG_CORE_RES))

DEPLUG_CORE_JS = $(wildcard deplug-core/*.js) $(wildcard deplug-core/components/*.js)
DEPLUG_CORE_MAIN_JS = $(wildcard deplug-core/*.main.js)
DEPLUG_CORE_JS_OUT = $(addprefix node_modules/,$(DEPLUG_CORE_MAIN_JS))

PLUGKIT_SRC = plugkit
PLUGKIT_DST = node_modules/plugkit

DISSECTOR_ESS = plugins/dissector/deplug-builtin-dissector-essentials

ROOLUP_EXTERN_BUILTIN = electron,deplug,$(shell node -p -e 'require("builtin-modules").join(",")')
ROOLUP_EXTERN = $(ROOLUP_EXTERN_BUILTIN),$(shell jq '.dependencies | keys | join(",")' package.json -r)

ESLINT = node_modules/.bin/eslint

ELECTRON_VERSION = $(shell jq '.devDependencies."electron-deplug"' package.json -r)
ELECTRON_MIRROR = https://s3-ap-northeast-1.amazonaws.com/deplug-build-junk/electron/v
ELECTRON_UNPACK = node_modules/deplug-helper
ELECTRON_IGNORE = "deplug-core","plugkit"

PACKAGER = node_modules/.bin/electron-packager

APPDMG = node_modules/.bin/appdmg

ifeq ($(OS),Windows_NT)
ELECTRON = node_modules\.bin\electron-deplug
ROLLUP = node_modules\.bin\rollup
else
ELECTRON = node_modules/.bin/electron-deplug
ROLLUP = node_modules/.bin/rollup
endif

all: build
	$(ELECTRON) --enable-logging .

build: $(DEPLUG_CORE_RES_OUT) $(DEPLUG_CORE_JS_OUT) plugkit

lint:
	$(ESLINT) $(DEPLUG_CORE_JS)

fix:
	$(ESLINT) --fix $(DEPLUG_CORE_JS)

plugkit:
	cp -r -f -p $(PLUGKIT_SRC) node_modules
	$(MAKE) -C $(PLUGKIT_DST)
	$(MAKE) -C $(DISSECTOR_ESS)

dmg:
	$(APPDMG) ci/appdmg.json out/deplug-darwin-amd64.dmg

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

winstaller:
	node ci/winstaller.js
	mv out/DeplugSetup.exe out/deplug-win-amd64.exe

pack: build
	$(PACKAGER) ./ --download.mirror=$(ELECTRON_MIRROR) \
	 						--asar.unpackDir=$(ELECTRON_UNPACK) \
							--ignore=$(ELECTRON_IGNORE) \
							--electron-version=$(ELECTRON_VERSION) \
							--out=./out --overwrite

$(DEPLUG_CORE_RES_OUT): $(DEPLUG_CORE_RES) $(DEPLUG_CORE)
	cp -r -f -p $(subst node_modules/,,$@) $@

$(DEPLUG_CORE_JS_OUT): $(DEPLUG_CORE_JS) $(DEPLUG_CORE)
	$(ROLLUP) $(subst node_modules/,,$@) -e $(ROOLUP_EXTERN) --format cjs --output $@

$(DEPLUG_CORE):
	@mkdir $(DEPLUG_CORE)

fmt:
	$(MAKE) fmt -C $(PLUGKIT_SRC)

clean:
	@rm -rf $(DEPLUG_CORE) $(PLUGKIT_DST)

.PHONY: all run build fix lint pack clean fmt plugkit dmg deb winstaller
