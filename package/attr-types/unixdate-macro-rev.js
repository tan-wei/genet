const moment = require('@deplug/moment.min')
module.exports = function ipv6 (attr) {
  const { value } = attr
  return moment(value).format()
}
