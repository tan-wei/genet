import genet from '@genet/api'
import m from 'mithril'

class AttrRenderer {
  view(vnode) {
    if (!vnode.attrs.attr) {
      return m('span')
    }
    return m('span', [vnode.attrs.attr.value.toString()])
  }
}

export default class Summary {
  view(vnode) {
    const { frame } = vnode.attrs
    const src = frame.query(`_.src`)
    const dst = frame.query(`_.dst`)
    if (!src || !dst) {
      return m('span', [])
    }
    const srcRenderer = genet.session.attrRenderer(src.type) || AttrRenderer
    const dstRenderer = genet.session.attrRenderer(dst.type) || AttrRenderer
    return m('span', [' ',
      m(srcRenderer, { attr: src }), ' ->', m(dstRenderer, { attr: dst })
    ])
  }
}
