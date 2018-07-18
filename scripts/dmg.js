#!/usr/bin/env node

const execa = require('execa')
const { readJSON } = require('fs-extra')

async function run() {
	const pkg = await readJSON('./package.json')
	const out = `genet-${process.platform}-${process.arch}-v${pkg.version}.dmg`
	await execa('npx', [
		'appdmg', 
		'scripts/appdmg.json', 
		`out/${out}`
	], {
		stdio: 'inherit'
	})
}

run()