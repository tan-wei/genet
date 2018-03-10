import { remote, ipcRenderer, shell } from 'electron'
import { HSplitter } from '../../lib/splitter'
import Menu from './menu'
import Resumer from '../../lib/resumer'
import Stack from './stack'
import env from '../../lib/env'
import fs from 'fs'
import m from 'mithril'
import path from 'path'
import touch from 'touch'

const { dialog } = remote
const windowId = remote.getCurrentWindow().id
const fields = Symbol('fields')
export default class WindowView {
  constructor () {
    this[fields] = {
      tabs: [],
      systemTabs: [
        {
          id: 'preferences',
          name: 'Preferences',
          src: 'preference.htm',
          argv: deplug.argv,
          loading: true,
          system: true,
          icon: 'fa-cog',
        },
        {
          id: 'packages',
          name: 'Packages',
          src: 'package.htm',
          argv: deplug.argv,
          loading: true,
          system: true,
          icon: 'fa-gift',
        }
      ],
      activeTab: 'preference',
      counter: 1,
    }
  }

  get tabs () {
    const { tabs, systemTabs } = this[fields]
    return tabs.concat(systemTabs)
  }

  get activeTab () {
    return this[fields].activeTab
  }

  set activeTab (id) {
    this[fields].activeTab = id
  }

  createPcapTab () {
    const { counter } = this[fields]
    const number = counter
    this[fields].counter += 1
    const id = `pcap-${number}`
    this.addTab({
      id,
      name: `Pcap ${number}`,
      src: 'pcap.htm',
      argv: deplug.argv.concat([
        `--resume=${Resumer.generateFileName()}`
      ]),
      loading: true,
    })
  }

  addTab (tab) {
    this[fields].tabs.push(tab)
    this[fields].activeTab = tab.id
    m.redraw()
  }

  removeTab (id) {
    this[fields].tabs = this[fields].tabs.filter((tab) => tab.id !== id)
  }

  oncreate () {
    const watcherFile = deplug.argv['deplug-dev-watch-file']
    if (watcherFile) {
      touch.sync(watcherFile)
      fs.watchFile(watcherFile, remote.getCurrentWindow().reload)
    }
    ipcRenderer.on('core:menu:action', (event, channel) => {
      document
        .querySelector('webview[active]')
        .getWebContents()
        .send(`${channel} #${windowId}`)
      remote
        .getCurrentWebContents()
        .send(`${channel} #${windowId}`)
    })
    deplug.action.global.on('core:tab:open-devtool', () => {
      document.querySelector('webview[active]').openDevTools()
    })
    deplug.action.global.on('core:tab:new-pcap', () => {
      this.createPcapTab()
    })
    deplug.action.global.on('core:tab:show-preferences', () => {
      this.activeTab = 'preferences'
      m.redraw()
    })
    deplug.action.global.on('core:tab:show-packages', () => {
      this.activeTab = 'packages'
      m.redraw()
    })
    deplug.action.global.on('core:window:new', () => {
      ipcRenderer.send('core:window:create')
    })
    deplug.action.global.on('core:file:browse-user-dir', () => {
      shell.showItemInFolder(env.userProfilePath)
    })
    deplug.action.global.on('core:pcap:uninstall-helper', () => {
      require('@deplug/osx-helper').uninstall()
    })
    deplug.action.global.on('core:file:import', () => {
      const files = dialog.showOpenDialog({
        properties: ['openFile'],
        filters: deplug.session.fileExtensions.importer,
      })
      if (typeof files !== 'undefined' && files.length > 0) {
        const [file] = files
        const id = `import-${file}`
        this.addTab({
          id,
          name: path.basename(file),
          src: 'pcap.htm',
          argv: deplug.argv.concat([
            `--import=${file}`,
            `--resume=${Resumer.generateFileName()}`
          ]),
          loading: true,
        })
      }
    })
    this.createPcapTab()
  }

  view () {
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
