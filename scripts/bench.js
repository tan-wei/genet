#!/usr/bin/env node

const { negatronBin } = require('./negatron')
const runAsNode = require('./run-as-node')
const glob = require('glob')

for (const file of process.argv.slice(2)) {
  runAsNode(file)
}
