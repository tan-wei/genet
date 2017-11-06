import env from '../lib/env'
import m from 'mithril'
import objpath from 'object-path'
export default class Plugin {
  constructor () {
    this.version = objpath.get(env.deplug, 'version', 'n/a')
    this.electronVersion =
      objpath.get(env.deplug, 'devDependencies.negatron', 'n/a')
  }
  view () {
    return [
      m('h1', ['Version']),
      m('h4', ['Deplug version']),
      m('span', [this.version]),
      m('h4', ['Negatron version']),
      m('span', [this.electronVersion])
    ]
  }
}
