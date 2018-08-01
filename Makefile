all: build
	npm run start

build:
	npm run build

test:
	npm test

fix:
	eslint --fix .

.PHONY: all build fix
