#!/usr/bin/env node
const { spawnSync } = require('child_process')
const { negatronBin } = require('./negatron')

module.exports = function (...args) {
  return spawnSync(negatronBin, args, {
    stdio: 'inherit',
    env: Object.assign({}, process.env, {
      ELECTRON_RUN_AS_NODE: "true",
      PLUGKIT_PCAP_DUMMY: "true"
    })
  })
}
