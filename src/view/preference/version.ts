import Env from '../../lib/env'
import native from '@genet/load-module'
import m from 'mithril'
import objpath from 'object-path'


export default class Plugin {
  private readonly version: string

  constructor() {
    this.version = objpath.get(Env.genet, 'version', 'n/a')
  }
  view(vnode) {
    return [
      m('h4', ['genet version']),
      m('span', [this.version]),
      m('h4', ['ABI version']),
      m('span', [native.version.abi])
    ]
  }
}
