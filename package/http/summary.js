const m = require('mithril')
class Summary {
  view (vnode) {
    const { layer } = vnode.attrs
    const value = ['']
    for (const attr of layer.attrs) {
      switch (attr.id) {
        case 'http.version':
          value.push(`HTTP/${attr.value.toPrecision(2)}`)
          break
        case 'http.path':
          value.push(` ${attr.value}`)
          break
        default:
          if (attr.id.startsWith('http.method.')) {
            value.push(` ${genet.session.tokenName(attr.id)}`)
          } else if (attr.id.startsWith('http.status.')) {
            const code = layer.attr('http.status').value
            value.push(` ${code} ${genet.session.tokenName(attr.id)}`)
          }
      }
    }
    return m('span', value)
  }
}
module.exports = Summary
