/* eslint-env mocha */

const assert = require('assert')
const execa = require('execa')
const path = require('path')
const generateVersionFile = require('./lib/version')
describe('gpm', () => {
  describe('list (no .version)', () => {
    it('should show "There are no installed packages."', () => {
      const { stdout } = execa.sync('node', ['./gpm.js', 'list', '-d', '_'], {
        stdio: 'pipe',
        env: { GENET_VERSION_FILE: '_' },
      })
      assert.equal(stdout, 'There are no installed packages.')
    })
  })
  describe('list --json (no .version)', () => {
    it('should show "[]"', () => {
      const { stdout } = execa.sync(
        'node', ['./gpm.js', 'list', '--json', '-d', '_'], {
          stdio: 'pipe',
          env: { GENET_VERSION_FILE: '_' },
        })
      assert.equal(stdout, '[]')
    })
  })
  describe('list', () => {
    it('should show the package list', () => {
      const dir = generateVersionFile()
      const { stdout } = execa.sync(
        'node', ['./gpm.js', 'list', '-d', path.join(dir, 'user')], {
          stdio: 'pipe',
          env: { GENET_VERSION_FILE: path.join(dir, '.version') },
        })
      assert.equal(stdout,
        `╔════════════════╤═══════╤═══════════╤═══════════╗
║ @genet/pkg     │ 0.1.0 │ :builtin: │           ║
╟────────────────┼───────┼───────────┼───────────╢
║ @genet/userpkg │ 0.1.0 │ test      │           ║
╚════════════════╧═══════╧═══════════╧═══════════╝
`
      )
    })
  })
  describe('list --json', () => {
    it('should show the package list as JSON', () => {
      const dir = generateVersionFile()
      const { stdout } = execa.sync(
        'node', ['./gpm.js', 'list', '--json', '-d', path.join(dir, 'user')], {
          stdio: 'pipe',
          env: { GENET_VERSION_FILE: path.join(dir, '.version') },
        })
      const data = JSON.parse(stdout)
      assert.deepEqual(data, [{
        metadata: {
          'name': '@genet/pkg',
          'version': '0.1.0',
          'engines': { 'genet': '*' },
          'genet': { 'components': [] },
        },
        'source': ':builtin:',
        'dir': path.join(dir, 'package/pkg'),
        'id': ':builtin::@genet/pkg:da4446a8',
      }, {
        metadata: {
          'name': '@genet/userpkg',
          'version': '0.1.0',
          'engines': { 'genet': '*' },
          'genet': { 'components': [] },
          '_gpm': { source: 'test' },
        },
        'source': 'test',
        'dir': path.join(dir, 'user/userpkg'),
        'id': 'test:@genet/userpkg:a646fbc7',
      }])
    })
  })
})
