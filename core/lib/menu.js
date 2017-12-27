import flatten from 'flat'
import objpath from 'object-path'
import { remote } from 'electron'
import template from './menu-template'
const { Menu } = remote
export default class MainMenu {
  get template () {
    return template
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

  enableContextMenu () {
    document.addEventListener('contextmenu', (event) => {

      const tmpl = [
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
    ]

      const menu = Menu.buildFromTemplate(tmpl)
      event.preventDefault()
      menu.popup(remote.getCurrentWindow())
    }, false)
  }
}
