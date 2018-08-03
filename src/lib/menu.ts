import { Disposable } from './disposable'
import flatten from 'flat'
import objpath from 'object-path'
import template from './menu-template'
const { remote } = require('electron')
const { Menu } = remote

const fields = Symbol('fields')
export default class MainMenu {
  constructor () {
    this[fields] = {
      contextMenuTemplates: new Set(),
      submenuTemplates: new Map(),
    }
  }

  get template () {
    const { submenuTemplates } = this[fields]
    const tmp = JSON.parse(JSON.stringify(template))
    const keys = Object.keys(flatten(template))
      .filter((key) => key.endsWith('.submenu'))
    for (const key of keys) {
      const val = objpath.get(tmp, key)
      if (typeof val === 'string') {
        const menu = submenuTemplates.get(val) || []
        objpath.set(tmp, key, menu)
      }
    }
    return tmp
  }

  get keymap () {
    const map = {}
    const keys = Object.keys(flatten(template))
      .filter((key) => key.endsWith('.accelerator'))
    for (const key of keys) {
      const accelerator = objpath.get(template, key)
        .replace('CommandOrControl', 'Command')
        .toLowerCase()
      const action = objpath.get(template,
        key.replace(/\.accelerator$/, '.action')) || ''
      const label = objpath.get(template,
        key.replace(/\.accelerator$/, '.label')) || ''
      map[accelerator] = [{
        action,
        selector: '(menu)',
        label,
      }]
    }
    return map
  }

  registerSubMenu (name: string, menu) {
    const { submenuTemplates } = this[fields]
    submenuTemplates.set(name, menu)
    return new Disposable(() => {
      submenuTemplates.delete(name)
    })
  }

  registerContextMenu (menu) {
    const { contextMenuTemplates } = this[fields]
    contextMenuTemplates.add(menu)
    return new Disposable(() => {
      contextMenuTemplates.delete(menu)
    })
  }

  showContextMenu (event: Event, menu = []) {
    const { contextMenuTemplates } = this[fields]
    let contextMenu: any[] = []
    if (menu.length > 0) {
      contextMenu = contextMenu.concat(menu, { type: 'separator' })
    }
    for (const tmpl of contextMenuTemplates) {
      if (event.target instanceof HTMLElement && 
        event.target.matches(tmpl.selector)) {
        contextMenu = contextMenu.concat(tmpl.menu, { type: 'separator' })
      }
    }

    contextMenu = contextMenu.concat([
      { role: 'undo' },
      { role: 'redo' },
      { type: 'separator' },
      { role: 'cut' },
      { role: 'copy' },
      { role: 'paste' },
      { role: 'pasteandmatchstyle' },
      { role: 'delete' },
      { role: 'selectall' },
      { type: 'separator' }
    ])

    event.preventDefault()
    event.stopPropagation()
    Menu.buildFromTemplate(contextMenu).popup(remote.getCurrentWindow())
  }

  enableContextMenu () {
    document.addEventListener('contextmenu', (event) => {
      this.showContextMenu(event)
    }, false)
  }
}
