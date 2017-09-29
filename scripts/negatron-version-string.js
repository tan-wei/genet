#!/usr/bin/env node

const jsonfile = require('jsonfile')
const path = require('path')

jsonfile.readFile(path.join(__dirname, '../package.json'), (err, obj) => {
  console.log(obj.devDependencies.negatron)
})
