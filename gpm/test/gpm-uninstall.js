/* eslint-env mocha */

const assert = require('assert')
const execa = require('execa')
const path = require('path')
const fs = require('fs-extra')
const generateVersionFile = require('./lib/version')
describe('gpm', () => {
  describe('uninstall', () => {
    it('should remove the package by name', () => {
      const dir = generateVersionFile()
      const pkgdir = path.join(dir, 'user', 'userpkg')
      assert.equal(fs.pathExistsSync(pkgdir), true)
      const { stdout } = execa.sync(
        'node', [
          './gpm.js', 'uninstall', '@genet/userpkg',
          '-d', path.join(dir, 'user')], {
          stdio: 'pipe',
          env: { GENET_VERSION_FILE: path.join(dir, '.version') },
        })
      assert.equal(stdout, 'removing @genet/userpkg ...')
      assert.equal(fs.pathExistsSync(pkgdir), false)
    })
    it('should remove the package by id', () => {
      const dir = generateVersionFile()
      const pkgdir = path.join(dir, 'user', 'userpkg')
      assert.equal(fs.pathExistsSync(pkgdir), true)

      const { stdout } = execa.sync(
        'node', [
          './gpm.js', 'uninstall', 'test:@genet/userpkg:a646fbc7',
          '-d', path.join(dir, 'user')], {
          stdio: 'pipe',
          env: { GENET_VERSION_FILE: path.join(dir, '.version') },
        })
      assert.equal(stdout, 'removing @genet/userpkg ...')
      assert.equal(fs.pathExistsSync(pkgdir), false)
    })
    it('should fail when the package is not installed', () => {
      const dir = generateVersionFile()
      assert.throws(() => {
        execa.sync(
          'node', [
            './gpm.js', 'uninstall', 'nopkg',
            '-d', path.join(dir, 'user')], {
            stdio: 'pipe',
            env: { GENET_VERSION_FILE: path.join(dir, '.version') },
          })
      }, (err) => err.stderr === 'package not found: nopkg\n')
    })
  })
})
