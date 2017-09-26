#!/usr/bin/env node
const childProcess = require('child_process')

let child = childProcess.spawn(process.argv[2], process.argv.slice(3), {
  stdio: 'inherit',
  env: Object.assign(process.env, {
    ELECTRON_RUN_AS_NODE: "true",
    PLUGKIT_PCAP_DUMMY: "true"
  })
})

child.on('close', (code) => {
  process.exit(code)
})
