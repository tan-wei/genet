#!/usr/bin/env node

const runAsNode = require('./run-as-node')

runAsNode('node_modules/mocha/bin/mocha', 'plugkit/test')
runAsNode('deplug-modules/core/test.main.js')
