const m = require('mithril')
const moment = require('@deplug/moment.min')
class UNIXDate {
  view (vnode) {
    const { value } = vnode.attrs.attr
    return m('span', [moment(value).format()])
  }
}
module.exports = UNIXDate
