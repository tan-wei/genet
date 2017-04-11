import { Deplugin } from 'deplugin'
import { Plugin } from 'deplug'
import PluginCard from './plugin-card'
import m from 'mithril'

export default class InstallView {
  constructor() {
    this.packages = [
      {'name': 'Now Loading...'}
    ]
  }

  oncreate() {
    let deplugin = new Deplugin()
    deplugin.cache().then((packages) => {
      this.packages = packages
      m.redraw()
      return deplugin.search()
    }).then((packages) => {
      this.packages = packages
      m.redraw()
    })
  }

  view(vnode) {
    return [
      <h1>Install Plugins </h1>
      ,
      <div>
        {
          this.packages.map((pkg) => {
            return m(PluginCard, {pkg: pkg})
          })
        }
      </div>
    ]
  }
}
