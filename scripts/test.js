#!/usr/bin/env node

const runAsNode = require('./run-as-node')

runAsNode('node_modules/mocha/bin/mocha', 'plugkit/test')
runAsNode('genet-modules/src/test.main.js')
