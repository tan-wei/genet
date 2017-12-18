#!/usr/bin/env node

const path = require('path')
const execa = require('execa')

const dpm = path.resolve(__dirname, '../node_modules/.bin/dpm')
const src = path.resolve(__dirname, '../package')

const proc = execa(dpm, ['update', src])
proc.stdout.pipe(process.stdout)
proc.stderr.pipe(process.stderr)
