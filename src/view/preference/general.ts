import ConfigList from './configlist'
import m from 'mithril'

export default class General {
  view(vnode) {
    return m(ConfigList, { prefix: vnode.attrs.prefix })
  }
}
