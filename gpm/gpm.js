#!/usr/bin/env node

const { version } = require('./lib/env')
const program = require('commander')
program
  .version(version)
  .usage('[command] [options]')
  .command('install', 'install package')
  .command('uninstall', 'uninstall package')
  .command('list', 'list local packages')
  .command('init', 'create a new package template')
  .parse(process.argv)
