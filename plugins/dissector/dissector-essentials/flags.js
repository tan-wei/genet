import m from 'mithril'
import {Token} from 'plugkit'
import {Session} from 'deplug'

export default class Flags {
  constructor() {

  }

  view(vnode) {
    const value = vnode.attrs.prop.value
    return <span>({ value })</span>
  }
}
