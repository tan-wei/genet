import genet from '@genet/api'
import m from 'mithril'

class AttrRenderer {
  view (vnode) {
    if (!vnode.attrs.attr) {
      return m('span')
    }
    return m('span', [vnode.attrs.attr.value.toString()])
  }
}

export default class Summary {
  view (vnode) {
    const { layer } = vnode.attrs
    const src = layer.attr(`${layer.id}.src`)
    const dst = layer.attr(`${layer.id}.dst`)
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
