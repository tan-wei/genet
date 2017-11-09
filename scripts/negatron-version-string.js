#!/usr/bin/env node

const jsonfile = require('jsonfile')
const path = require('path')
const semver = require('semver')

jsonfile.readFile(path.join(__dirname, '../package.json'), (err, obj) => {
  const ver = semver.parse(obj.devDependencies.negatron)
  console.log(`${ver.major}.${ver.minor}.${ver.patch}`)
})
