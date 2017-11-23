import { remote } from 'electron'
import template from './menu-template'
const { Menu } = remote
export default class MainMenu {
  get template () {
    return template
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
      { role: 'selectall' }]

      const menu = Menu.buildFromTemplate(tmpl)
      event.preventDefault()
      menu.popup(remote.getCurrentWindow())
    }, false)
  }
}
