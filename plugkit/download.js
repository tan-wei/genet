#!/usr/bin/env node
const fs = require('fs')
const path = require('path')
const mkpath = require('mkpath')
const request = require('request')

const version = require('./package.json').version
const platform = process.platform === 'darwin' ? 'osx' : process.platform
const architecture = 'x64'
const mirrorPath = 'https://s3-ap-northeast-1.amazonaws.com/deplug-build-junk/plugkit'

const libraryDir = path.join(__dirname, 'build', 'Release')
const libraryPath = path.join(libraryDir, 'plugkit.node')

const archivePath = `${platform}/${architecture}/${version}`
const archiveUrl = `${mirrorPath}/${archivePath}/plugkit.node`

mkpath.sync(libraryDir)

request(archiveUrl)
.on('error', (err) => {
  process.exit(1)
})
.on('end', () => {
  process.exit(0)
})
.pipe(fs.createWriteStream(libraryPath))
