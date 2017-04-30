import { EventEmitter } from 'events'
import internal from './internal'
import objpath from 'object-path'
import { remote } from 'electron'
import throttle from 'lodash.throttle'

const handlers = []

const reload = throttle(() => {
  const root = { 'Electron': {}, }
  const handlerSymbol = Symbol('handler')
  for (const handler of handlers) {
    if (!objpath.has(root, handler.path)) {
      objpath.set(root, handler.path, { [handlerSymbol]: handler, })
    }
  }

  function crateMenuItem (name, object) {
    if (handlerSymbol in object) {
      const { accelerator, selector, click, } = object[handlerSymbol]
      const item = {
        label: name,
        selector,
        accelerator,
        click,
      }
      return new remote.MenuItem(item)
    }

    return new remote.MenuItem({
      label: name,
      type: 'submenu',
      submenu: Object.keys(object)
        .map((key) => crateMenuItem(key, object[key])),
    })
  }

  const menu = new remote.Menu()
  for (const name in root) {
    menu.append(crateMenuItem(name, root[name]))
  }

  if (process.platform === 'darwin') {
    remote.Menu.setApplicationMenu(menu)
  } else {
    remote.getCurrentWindow().setMenu(menu)
  }
})
export default class Menu extends EventEmitter {
  static registerHandler (handler) {
    handlers.push(handler)
    reload()
  }

  constructor () {
    super()
    internal(this).handlers = {}
    internal(this).mainHadlers = {}
    internal(this).mainPriorities = {}
  }

  register (name, handler, priority = 0) {
    if (!(name in internal(this).handlers)) {
      internal(this).handlers[name] = []
    }
    internal(this).handlers[name].push({
      handler,
      priority,
    })
    internal(this).handlers[name]
      .sort((first, second) => first.priority - second.priority)
  }

  unregister (name, handler) {
    if (!(name in internal(this).handlers)) {
      internal(this).handlers[name] = []
    }
    internal(this).handlers[name] =
      internal(this).handlers[name].filter((item) => item.handler !== handler)
  }

  registerMain (name, handler, priority = 0) {
    if (!(name in internal(this).mainHadlers)) {
      internal(this).mainHadlers[name] = []
    }
    internal(this).mainHadlers[name].push({
      handler,
      priority,
    })
    internal(this).mainHadlers[name]
      .sort((first, second) => first.priority - second.priority)
    this.updateMainMenu()
  }

  unregisterMain (name, handler) {
    if (!(name in internal(this).mainHadlers)) {
      internal(this).mainHadlers[name] = []
    }
    internal(this).mainHadlers[name] =
      internal(this).mainHadlers[name]
        .filter((item) => item.handler !== handler)
    this.updateMainMenu()
  }

  setMainPriority (name, priority) {
    internal(this).mainPriorities[name] = priority
  }

  updateMainMenu () {
    const root = new remote.Menu()
    const keys = Object.keys(internal(this).mainHadlers)
    keys.sort((first, second) => (internal(this).mainPriorities[second] || 0) -
        (internal(this).mainPriorities[first] || 0))
    for (const key of keys) {
      let menu = new remote.Menu()
      const list = internal(this).mainHadlers[key]
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
    if (name in internal(this).handlers) {
      let menu = new remote.Menu()
      const list = internal(this).handlers[name]
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
