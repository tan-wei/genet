#!/usr/bin/env node

const path = require('path')
const execa = require('execa')
const fs = require('fs')
const os = require('os')
const jsonfile = require('jsonfile')

const version = jsonfile.readFileSync(
  path.join(__dirname, '../package.json')).devDependencies.negatron

const dpm = path.resolve(__dirname, '../node_modules/.bin/dpm')
const src = path.resolve(__dirname, '../package')

const proc = execa(dpm, ['update', '--negatron', version, src])
proc.stdout.pipe(process.stdout)
proc.stderr.pipe(process.stderr)
