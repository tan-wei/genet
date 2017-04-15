import { Deplugin } from 'deplugin'
import { Plugin } from 'deplug'
import PluginCard from './plugin-card'
import m from 'mithril'

export default class InstallView {
  constructor() {
    this.packages = []
    this.loading = false
    this.error = false
  }

  oncreate() {
    let deplugin = new Deplugin()
    deplugin.cache().then((packages) => {
      this.packages = packages
      this.loading = true
      m.redraw()
      return deplugin.search()
    }).then((packages) => {
      this.packages = packages
      this.loading = false
      m.redraw()
    }).catch(() => {
      this.loading = false
      this.error = true
      m.redraw()
    })
  }

  view(vnode) {
    return [
      <h1>Install Plugins </h1>,
      <p style={{visibility: (this.loading || this.error) ? 'visible' : 'hidden'}}>
      <i class={this.error ? 'fa fa-exclamation-triangle' : 'fa fa-refresh fa-spin'}>
      </i>{ this.error ? ' Error' : ' Fetching package list...' }</p>,
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
