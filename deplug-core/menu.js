import { EventEmitter } from 'events'
import { remote } from 'electron'

const handlers = Symbol('handlers')
const mainHadlers = Symbol('mainHadlers')
const mainPriorities = Symbol('mainPriorities')
export default class Menu extends EventEmitter {
  constructor () {
    super()
    this[handlers] = {}
    this[mainHadlers] = {}
    this[mainPriorities] = {}
  }

  register (name, handler, priority = 0) {
    if (!(name in this[handlers])) {
      this[handlers][name] = []
    }
    this[handlers][name].push({
      handler,
      priority,
    })
    this[handlers][name]
      .sort((first, second) => first.priority - second.priority)
  }

  unregister (name, handler) {
    if (!(name in this[handlers])) {
      this[handlers][name] = []
    }
    this[handlers][name] =
      this[handlers][name].filter((item) => item.handler !== handler)
  }

  registerMain (name, handler, priority = 0) {
    if (!(name in this[mainHadlers])) {
      this[mainHadlers][name] = []
    }
    this[mainHadlers][name].push({
      handler,
      priority,
    })
    this[mainHadlers][name]
      .sort((first, second) => first.priority - second.priority)
    this.updateMainMenu()
  }

  unregisterMain (name, handler) {
    if (!(name in this[mainHadlers])) {
      this[mainHadlers][name] = []
    }
    this[mainHadlers][name] =
      this[mainHadlers][name].filter((item) => item.handler !== handler)
    this.updateMainMenu()
  }

  setMainPriority (name, priority) {
    this[mainPriorities][name] = priority
  }

  updateMainMenu () {
    const root = new remote.Menu()
    const keys = Object.keys(this[mainHadlers])
    keys.sort((first, second) => (this[mainPriorities][second] || 0) -
        (this[mainPriorities][first] || 0))
    for (const key of keys) {
      let menu = new remote.Menu()
      const list = this[mainHadlers][key]
      list.forEach((item, index) => {
        menu = Reflect.apply(item.handler, this, [menu])
        if (index < list.length - 1) {
          menu.append(new remote.MenuItem({ type: 'separator', }))
        }
      })
      if (menu.items.length > 0) {
        const item = {
          label: key,
          submenu: menu,
          type: 'submenu',
        }
        switch (key) {
          case 'Help':
            item.role = 'help'
            break
          case 'Window':
            item.role = 'window'
            break
          default:

        }
        root.append(new remote.MenuItem(item))
      }
    }

    if (process.platform === 'darwin') {
      remote.Menu.setApplicationMenu(root)
    } else {
    remote.getCurrentWindow().setMenu(root)
    }
  }

  popup (name, self, browserWindow, option = {}) {
    if (name in this[handlers]) {
      let menu = new remote.Menu()
      const list = this[handlers][name]
      list.forEach((item, index) => {
        menu = Reflect.apply(item.handler, self, [menu, option.event])
        if (index < list.length - 1) {
          menu.append(new remote.MenuItem({ type: 'separator', }))
        }
      })
      menu.popup(browserWindow, option.x, option.y)
    }
  }
}
