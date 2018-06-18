all:
	npm run watch

build:
	npm run build

test:
	npm test

fix:
	eslint --fix .

.PHONY: all build fix
