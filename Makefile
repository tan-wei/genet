all:
	npm run watch

build:
	npm run build

test:
	npm test

fix:
	eslint --fix .

fmt:
	-clang-format -i plugkit/**/*.{cpp,hpp,h,c} package/**/*.{cpp,hpp}

.PHONY: all build fix fmt
