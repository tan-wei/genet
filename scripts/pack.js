#!/usr/bin/env node

const { negatronVersion } = require('./negatron')
const path = require('path')
const { ensureDir, copy, remove } = require('fs-extra')
const execa = require('execa')
const modclean = require('modclean')
const glob = require('glob')
const packager = require('electron-packager')

const outDir = path.join(__dirname, '../out')
const outSrcDir = path.join(outDir, 'src')
const outDistDir = path.join(outDir, 'dist')

const options = {
  dir: outSrcDir,
  download: { mirror: 'https://storage.googleapis.com/cdn-genet-app/electron/v' },
  asar: { unpackDir: '{node_modules/nan,package,genet-modules/bin}' },
  icon: 'images/genet',
  ignore: /^(src|lib|scripts|debian|images)/,
  electronVersion: negatronVersion,
  win32metadata: {
    CompanyName: 'genet',
    FileDescription: 'General Purpose Network Analyzer',
    OriginalFilename: 'genet.exe',
    ProductName: 'genet',
    InternalName: 'genet',
  },
  out: outDistDir,
  overwrite: true,
}

async function run() {
	await ensureDir(outSrcDir)
	await copy(path.join(__dirname, '../package.json'), path.join(outSrcDir, 'package.json'))
	await copy(path.join(__dirname, '../package-lock.json'), path.join(outSrcDir, 'package-lock.json'))
	await copy(path.join(__dirname, '../genet-modules'), path.join(outSrcDir, 'genet-modules'))
	await copy(path.join(__dirname, '../package'), path.join(outSrcDir, 'package'))

	const waste = await glob.sync('package/*/crates/**/*', {
		cwd: outSrcDir,
		absolute: true,
		nodir: true,
		ignore: ['**/target/release/*']
	})
	await Promise.all(waste.map(file => remove(file)))

    await execa.shell('npm i', {
      cwd: outSrcDir,
      stdio: 'inherit',
      env: { NODE_ENV: 'production' }
    })

	const mc = modclean({
	  cwd: outSrcDir
	})
	await mc.clean()

	try {
		await packager(options)
	} catch (err) {
		console.warn(err)
		process.exit(1)		
	}
}

run()