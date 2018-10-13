const axios = require('axios')
const execa = require('execa')
const tempy = require('tempy')
const { parse } = require('node-html-parser')
module.exports = {
  async isSupported (url) {
    if (url.endsWith('.git')) {
      return true
    }

    try {
      const { data } = await axios.get(url, { responseType: 'document' })
      const meta = parse(data).querySelectorAll('meta')
        .find((tag) => tag.attributes.name === 'go-import')
      if (meta) {
        return true
      }
    } catch (err) {
      return false
    }

    return false
  },
  async fetch (url) {
    const dir = tempy.directory()
    try {
      await execa('git', ['--version'])
    } catch (err) {
      throw new Error('Git command is not found: ' +
        'You need git to install the package from git repository.')
    }
    await execa('git', ['clone', '--depth=1', '--recursive', url, dir],
      { stdio: 'inherit' })
    return dir
  },
}
