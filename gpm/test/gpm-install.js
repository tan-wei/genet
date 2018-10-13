/* eslint-env mocha */

const assert = require('assert')
const execa = require('execa')
const path = require('path')
const fs = require('fs-extra')
const generateVersionFile = require('./lib/version')
describe('gpm', () => {
  describe('install', () => {
    it('should install the package', () => {
      const dir = generateVersionFile()
      execa.sync(
        'node', [
          './gpm.js', 'install', 'test/pkg/arp',
          '-d', path.join(dir, 'user')], {
          stdio: 'inherit',
          env: { GENET_VERSION_FILE: path.join(dir, '.version') },
        })
      const targetdir = path.join(dir, 'user', 'arp-test/target')
      assert.equal(fs.pathExistsSync(targetdir), true)
    })
  })
  describe('install', () => {
    it('should install the package (from git)', () => {
      const dir = generateVersionFile()
      execa.sync(
        'node', [
          './gpm.js', 'install', 'https://github.com/genet-app/arp',
          '-d', path.join(dir, 'user')], {
          stdio: 'inherit',
          env: { GENET_VERSION_FILE: path.join(dir, '.version') },
        })
      const targetdir = path.join(dir, 'user', '@genet/arp/target')
      assert.equal(fs.pathExistsSync(targetdir), true)
    })
  })
  describe('install', () => {
    it('should fail when the package directory does not exist', () => {
      const dir = generateVersionFile()
      assert.throws(() => {
        execa.sync(
          'node', [
            './gpm.js', 'install', 'test/pkg/null',
            '-d', path.join(dir, 'user')], {
            stdio: 'inherit',
            env: { GENET_VERSION_FILE: path.join(dir, '.version') },
          })
      })
    })
  })
  describe('install', () => {
    it('should fail when the package.json does not exist', () => {
      const dir = generateVersionFile()
      assert.throws(() => {
        execa.sync(
          'node', [
            './gpm.js', 'install', 'test/pkg/nopkg',
            '-d', path.join(dir, 'user')], {
            stdio: 'inherit',
            env: { GENET_VERSION_FILE: path.join(dir, '.version') },
          })
      })
    })
  })
  describe('install', () => {
    it('should fail when engines.genet does not exist in package.json', () => {
      const dir = generateVersionFile()
      assert.throws(() => {
        execa.sync(
          'node', [
            './gpm.js', 'install', 'test/pkg/nopkg',
            '-d', path.join(dir, 'user')], {
            stdio: 'inherit',
            env: { GENET_VERSION_FILE: path.join(dir, '.version') },
          })
      })
    })
  })
})
