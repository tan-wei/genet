const m = require('mithril')
class IPv6Addr {
  view (vnode) {
    const { value } = vnode.attrs.prop
    const array = [
      (value[0] << 8) | value[1],
      (value[2] << 8) | value[3],
      (value[4] << 8) | value[5],
      (value[6] << 8) | value[7],
      (value[8] << 8) | value[9],
      (value[10] << 8) | value[11],
      (value[12] << 8) | value[13],
      (value[14] << 8) | value[15]
    ]
    let addr = array.map((byte) => byte.toString(16)).join(':')
    const seq = /:?\b0(:0)*:?/g
    let maxSeq = ''
    let maxIndex = 0
    let result = null
    while ((result = seq.exec(addr)) !== null) {
      if (result[0].length > maxSeq.length) {
        [maxSeq] = result
        maxIndex = result.index
      }
    }
    if (maxSeq.length) {
      addr = `${addr.substr(0, maxIndex)
         }::${addr.substr(maxSeq.length + maxIndex)}`
    }
    return m('span', [addr])
  }
}
module.exports = IPv6Addr
