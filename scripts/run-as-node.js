#!/usr/bin/env node
const { spawnSync } = require('child_process')
const { electronBin } = require('./electron')

module.exports = function (...args) {
  return spawnSync(electronBin, args, {
    stdio: 'inherit',
    env: {
      ...process.env,
      ELECTRON_RUN_AS_NODE: "true",
      PLUGKIT_PCAP_DUMMY: "true"
    }
  })
}
