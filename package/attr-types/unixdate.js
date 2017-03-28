const m = require('mithril')
const moment = require('moment')
class UNIXDate {
  view (vnode) {
    const { value } = vnode.attrs.attr
    return m('span', [moment(value).format()])
  }
}
module.exports = UNIXDate
