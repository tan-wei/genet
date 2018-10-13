const isLocal = require('is-local-path')
module.exports = {
  async isSupported (url) {
    return isLocal(url)
  },
  async fetch (url) {
    return url
  },
}
