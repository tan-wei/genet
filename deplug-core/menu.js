import { EventEmitter } from 'events'
import internal from './internal'
import objpath from 'object-path'
import { remote } from 'electron'
import throttle from 'lodash.throttle'

const handlers = []

const reload = throttle(() => {
  const root = {}
  if (process.platform === 'darwin') {
    root[remote.app.getName()] = {}
  }
  const handlerSymbol = Symbol('handler')
  const orderSymbol = Symbol('order')

  let index = 0
  for (const handler of handlers) {
    for (let num = 0; num < handler.path.length; num += 1) {
      const path = handler.path.slice(0, -num)
      if (!objpath.has(root, path)) {
        objpath.set(root, path, { [orderSymbol]: index })
      }
      index += 1
    }
    objpath.set(root, handler.path, {
      [handlerSymbol]: handler,
      [orderSymbol]: index,
    })
    index += 1
  }

  function crateMenuItem (name, object) {
    if (handlerSymbol in object) {
      const item = Object.assign({ label: name }, object[handlerSymbol])
      return new remote.MenuItem(item)
    }

    const keys = Object.keys(object)
    keys.sort((lhs, rhs) => (object[lhs][orderSymbol] || 0) -
      (object[rhs][orderSymbol] || 0))

    return new remote.MenuItem({
      label: name,
      type: 'submenu',
      submenu: keys.map((key) => crateMenuItem(key, object[key])),
    })
  }

  const menu = new remote.Menu()
  const keys = Object.keys(root)
  keys.sort((lhs, rhs) => (root[lhs][orderSymbol] || 0) -
    (root[rhs][orderSymbol] || 0))
  for (const name of keys) {
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

  popup (name, self, browserWindow, option = {}) {
    if (name in internal(this).handlers) {
      let menu = new remote.Menu()
      const list = internal(this).handlers[name]
      list.forEach((item, index) => {
        menu = Reflect.apply(item.handler, self, [menu, option.event])
        if (index < list.length - 1) {
          menu.append(new remote.MenuItem({ type: 'separator' }))
        }
      })
      menu.popup(browserWindow, option.x, option.y)
    }
  }
}

if (process.platform === 'darwin') {
  remote.getCurrentWindow().on('focus', reload)
}
