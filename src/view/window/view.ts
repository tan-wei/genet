import Env from '../../lib/env'
import { HSplitter } from '../../lib/splitter'
import Menu from './menu'
import Resumer from '../../lib/resumer'
import Stack from './stack'
import chokidar from 'chokidar'
import fs from 'fs-extra'
import flatten from 'lodash.flatten'
import genet from '@genet/api'
import m from 'mithril'
import path from 'path'
const { remote, ipcRenderer, shell } = require('electron')
const { dialog } = remote

interface Tab {
  id: string
  name: string
  src: string
  argv: object
  loading: boolean
  system: boolean
  icon: string
}

const windowId = remote.getCurrentWindow().id
export default class WindowView {
  private _tabs: Tab[]
  private _systemTabs: Tab[]
  private _activeTab: string
  private _counter: number
  constructor() {
    this._tabs = []
    this._systemTabs = [
      {
        id: 'preferences',
        name: 'Preferences',
        src: 'preference.htm',
        argv: { ...genet.argv },
        loading: true,
        system: true,
        icon: 'fa-cog',
      },
      {
        id: 'packages',
        name: 'Packages',
        src: 'package.htm',
        argv: { ...genet.argv },
        loading: true,
        system: true,
        icon: 'fa-gift',
      }
    ]
    this._activeTab = 'preference'
    this._counter = 1
  }

  get tabs() {
    return this._tabs.concat(this._systemTabs)
  }

  get activeTab() {
    return this._activeTab
  }

  set activeTab(id) {
    this._activeTab = id
  }

  createDisplayTab() {
    const number = this._counter
    this._counter += 1
    const id = `display-${number}`
    this.addTab({
      id,
      name: `Session ${number}`,
      src: 'display.htm',
      argv: { ...genet.argv, resume: Resumer.generateFileName() },
      loading: true,
    })
  }

  addTab(tab) {
    this._tabs.push(tab)
    this._activeTab = tab.id
    m.redraw()
  }

  removeTab(id) {
    this._tabs = this._tabs.filter((tab) => tab.id !== id)
  }

  private registerTabKeybinds(index: number) {
    genet.action.global.on(`core:tab:active:${index}`, () => {
      if (this._tabs.length >= index) {
        this._activeTab = this._tabs[index - 1].id
        m.redraw()
      }
    })
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
    genet.action.global.on('core:help:documentation', () => {
      shell.openExternal('https://docs.genet.app/')
    })
    genet.action.global.on('core:tab:open-devtool', () => {
      const webview = document.querySelector('webview[active]') as any
      if (webview !== null) {
        webview.openDevTools()
      }
    })
    genet.action.global.on('core:tab:new-pcap', () => {
      this.createDisplayTab()
    })
    genet.action.global.on('core:tab:active:preferences', () => {
      this.activeTab = 'preferences'
      m.redraw()
    })
    genet.action.global.on('core:tab:active:packages', () => {
      this.activeTab = 'packages'
      m.redraw()
    })
    for (let i = 1; i <= 9; ++i) {
      this.registerTabKeybinds(i)
    }
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
      const files = dialog.showOpenDialog({
        properties: ['openFile'],
        filters: [{
          name: 'Pcap Files',
          extensions: ['pcap'],
        },{
          name: 'All Files',
          extensions: [],
        }],
      })
      if (typeof files !== 'undefined' && files.length > 0) {
        const [file] = files
        const id = `import-${Date.now()}`
        this.addTab({
          id,
          name: path.basename(file),
          src: 'display.htm',
          argv: {
            ...genet.argv,
            import: file,
            resume: Resumer.generateFileName()
          },
          loading: true,
        })
      }
    })
    const tabRealodingFile = path.join(Env.userPath, '.reload')
    fs.ensureFile(tabRealodingFile, () => {
      chokidar.watch(tabRealodingFile, { ignoreInitial: true }).on('all', () => {
        if (genet.config.get('_.dev.tabReloading', false)) {
          const webview = document.querySelectorAll('webview')
          webview.forEach((view: any) => {
            view.setAttribute('loading', '')
            view.executeJavaScript(`
              require('@genet/api').action.global.emit('core:tab:reload')
            `)
            view.addEventListener('did-finish-load', () => {
              view.removeAttribute('loading')
            }, { once: true })
          })
        }
      })
    })

    this.createDisplayTab()
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
