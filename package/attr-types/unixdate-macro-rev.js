const moment = require('moment')
module.exports = function ipv6 (attr) {
  const { value } = attr
  return moment(value).format()
}
