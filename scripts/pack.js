#!/usr/bin/env node

const { negatronVersion } = require('./negatron')
const path = require('path')
const { ensureDir, copy, remove } = require('fs-extra')
const execa = require('execa')
const glob = require('glob')
const packager = require('electron-packager')
const modclean = require('modclean')({})

const outDir = path.join(__dirname, '../out')
const outSrcDir = path.join(outDir, 'src')
const outDistDir = path.join(outDir, 'dist')

const options = {
  dir: outSrcDir,
  download: { mirror: 'https://storage.googleapis.com/cdn-genet-app/electron/v' },
  asar: { unpackDir: '{package,genet_modules/bin}' },
  icon: 'images/genet',
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
	await copy(path.join(__dirname, '../genet_modules'), path.join(outSrcDir, 'genet_modules'))
	await copy(path.join(__dirname, '../package'), path.join(outSrcDir, 'package'))

	const waste = await glob.sync('package/*/crates/**/*', {
		cwd: outSrcDir,
		absolute: true,
		nodir: true,
		ignore: ['**/target/release/*']
	})
	await Promise.all(waste.map(file => remove(file)))

  await execa('npm', ['install'], {
    cwd: outSrcDir,
    stdio: 'inherit',
    env: { NODE_ENV: 'production' }
  })

  await modclean.clean();

	try {
		await packager(options)
	} catch (err) {
		console.warn(err)
		process.exit(1)
	}
}

run()
