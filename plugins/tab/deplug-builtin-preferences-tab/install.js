import { Deplugin } from 'deplugin'
import { Plugin, Config } from 'deplug'
import PluginCard from './plugin-card'
import m from 'mithril'
import moment from 'moment'

export default class InstallView {
  constructor() {
    this.packages = []
    this.loading = false
    this.deplugin = new Deplugin(Config.deplug.version)
  }

  oncreate() {
    this.deplugin.cache().then((packages) => {
      this.packages = packages
      m.redraw()
      if (moment(this.deplugin.lastUpdated)
        .isBefore(moment().subtract(1, 'hours'))) {
        this.update()
      }
    })
  }

  update() {
    if (!this.loading) {
      this.loading = true
      this.deplugin.search().then((packages) => {
        this.packages = packages
        this.loading = false
        m.redraw()
      }).catch(() => {
        this.loading = false
        m.redraw()
      })
    }
  }

  view(vnode) {
    const lastUpdated = moment(this.deplugin.lastUpdated).fromNow()
    return [
      <h1>Install Plugins </h1>,
      <p style={{display: this.loading ? 'block' : 'none'}}>
      <i class="fa fa-refresh fa-spin"></i> Fetching package list...</p>,
      <p style={{display: !this.loading ? 'block' : 'none'}}>
      <a href="javascript:void(0)" onclick={()=>{ this.update() }}>
      <i class="fa fa-check"></i>{` Last updated: ${lastUpdated}`}</a>
      </p>,
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
