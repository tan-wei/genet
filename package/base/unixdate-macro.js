const moment = require('moment')
module.exports = function unixdate (str) {
  const date = moment(str, moment.ISO_8601)
  if (!date.isValid()) {
    return null
  }
  return JSON.stringify(date.valueOf())
}
