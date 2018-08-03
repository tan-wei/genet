import Env from '../../lib/env'
import { HSplitter } from '../../lib/splitter'
import Menu from './menu'
import Stack from './stack'
import flatten from 'lodash.flatten'
import genet from '@genet/api'
import m from 'mithril'
import path from 'path'
const { remote, ipcRenderer, shell } = require('electron')
const { dialog } = remote

const windowId = remote.getCurrentWindow().id
const fields = Symbol('fields')
export default class WindowView {
  constructor() {
    this[fields] = {
      tabs: [],
      systemTabs: [
        {
          id: 'preferences',
          name: 'Preferences',
          src: 'preference.htm',
          argv: genet.argv,
          loading: true,
          system: true,
          icon: 'fa-cog',
        },
        {
          id: 'packages',
          name: 'Packages',
          src: 'package.htm',
          argv: genet.argv,
          loading: true,
          system: true,
          icon: 'fa-gift',
        }
      ],
      activeTab: 'preference',
      counter: 1,
    }
  }

  get tabs() {
    const { tabs, systemTabs } = this[fields]
    return tabs.concat(systemTabs)
  }

  get activeTab() {
    return this[fields].activeTab
  }

  set activeTab(id) {
    this[fields].activeTab = id
  }

  createPcapTab() {
    const { counter } = this[fields]
    const number = counter
    this[fields].counter += 1
    const id = `display-${number}`
    this.addTab({
      id,
      name: `Session ${number}`,
      src: 'display.htm',
      argv: genet.argv,
      loading: true,
    })
  }

  addTab(tab) {
    this[fields].tabs.push(tab)
    this[fields].activeTab = tab.id
    m.redraw()
  }

  removeTab(id) {
    this[fields].tabs = this[fields].tabs.filter((tab) => tab.id !== id)
  }

  oncreate() {
    ipcRenderer.on('core:menu:action', (event, channel) => {
      const webview = document.querySelector('webview[active]') as any
      if (webview !== null) {
        webview.getWebContents()
          .send(`${channel} #${windowId}`)
      }
      remote
        .getCurrentWebContents()
        .send(`${channel} #${windowId}`)
    })
    genet.action.global.on('core:tab:open-devtool', () => {
      const webview = document.querySelector('webview[active]') as any
      if (webview !== null) {
        webview.openDevTools()
      }
    })
    genet.action.global.on('core:tab:new-pcap', () => {
      this.createPcapTab()
    })
    genet.action.global.on('core:tab:show-preferences', () => {
      this.activeTab = 'preferences'
      m.redraw()
    })
    genet.action.global.on('core:tab:show-packages', () => {
      this.activeTab = 'packages'
      m.redraw()
    })
    genet.action.global.on('core:window:new', () => {
      ipcRenderer.send('core:window:create')
    })
    genet.action.global.on('core:menu:reload', () => {
      ipcRenderer.send('core:menu:reload', windowId)
    })
    genet.action.global.on('core:file:browse-user-dir', () => {
      shell.showItemInFolder(Env.userProfilePath)
    })
    genet.action.global.on('core:file:import', () => {
      const filters = flatten([...genet.session.fileReaders]
        .map((reader) => reader.filters))
      const files = dialog.showOpenDialog({
        properties: ['openFile'],
        filters,
      })
      if (typeof files !== 'undefined' && files.length > 0) {
        const [file] = files
        const id = `import-${Date.now()}`
        this.addTab({
          id,
          name: path.basename(file),
          src: 'display.htm',
          argv: genet.argv.concat([
            `--import=${file}`
          ]),
          loading: true,
        })
      }
    })
    this.createPcapTab()
  }

  view() {
    return [
      m(HSplitter, {
        left: Menu,
        right: Stack,
        parent: this,
        width: 180,
        workspace: '_.menuWidth',
      })
    ]
  }
}
