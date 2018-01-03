all:
	npm run watch

build:
	npm run build

test:
	npm test

docs:
	mkdir -p out
	cp -r -f -p docs/. out/docs
	@node scripts/generate-docs.js c-parser.js ./snippet $(PLUGKIT_HEADERS) out/docs/diss-api-c.md
	@node scripts/generate-docs.js js-parser.js ./snippet $(PLUGKIT_JS_FILES) out/docs/diss-api-js.md
	@node scripts/generate-docs.js js-parser.js ./snippet $(DEPLUG_CORE_LIB_JS) out/docs/core-api-js.md
	@node scripts/generate-token-docs.js package/token-wellknown/tokens.json out/docs/well-known-tokens.md

docs-serve: docs
	docsify serve ./out/docs

fix:
	eslint --fix .

fmt:
	clang-format -i plugkit/**/*.{cpp,hpp,h,c}

.PHONY: all build fix fmt docs docs-serve
