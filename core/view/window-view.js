import WebView from '../lib/webview'
import m from 'mithril'
import path from 'path'
import { remote } from 'electron'

const { dialog } = remote
export default class WindowView {
  constructor () {
    this.tabs = [
    {
      id: 'pcap-1',
      name: 'Pcap 1',
      src: 'pcap.htm',
      argv: deplug.argv,
      loading: true,
    }, {
      id: 'preference',
      name: 'Preferences',
      src: 'preference.htm',
      argv: deplug.argv,
      loading: true,
      icon: 'fa-cog',
    }, {
      id: 'log',
      name: 'Logs',
      src: 'log.htm',
      argv: deplug.argv,
      loading: true,
      icon: 'fa-book',
    }]
    this.activeTab = 'pcap-1'
  }

  oncreate () {
    deplug.action.global.on('core:tab:open-devtool', () => {
      document.querySelector('webview[active]').openDevTools()
    })
    deplug.action.global.on('core:file:import', () => {
      const files = dialog.showOpenDialog({
        properties: ['openFile'],
        filters: [
          {
            name: 'Libpcap File',
            extensions: ['pcap'],
          }],
        })
      if (files.length > 0) {
        const [file] = files
        const id = `import-${file}`
        this.tabs.unshift({
          id,
          name: path.basename(file),
          src: 'pcap.htm',
          argv: deplug.argv.concat([`--import=${file}`]),
          loading: true,
        })
        this.activeTab = id
        m.redraw()
      }
    })
  }

  view () {
    const userTabs = this.tabs.map((tab) => m('ul', [
        m('li', [
          m('a', {
            onclick: () => {
             this.activeTab = tab.id
            },
            active: this.activeTab === tab.id,
          }, [m('i', { class: `fa ${tab.icon}` }), ' ', tab.name])
        ])
      ]))
    return [
      m('nav', userTabs),
      m('main', this.tabs.map((tab) => m(WebView, {
        tab,
        key: tab.id,
        active: this.activeTab === tab.id,
      })))
    ]
  }
}
