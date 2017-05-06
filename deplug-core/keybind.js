import { EventEmitter } from 'events'
import Mousetrap from 'mousetrap'
import internal from './internal'

export default class Keybind extends EventEmitter {
  constructor (profile) {
    super()
    internal(this).profile = profile
    internal(this).builtinCommands = {}
    internal(this).commands = {}
  }

  bind (command, selector, act) {
    if (!(command in internal(this).builtinCommands)) {
      internal(this).builtinCommands[command] = {}
    }
    internal(this).builtinCommands[command][selector] = act
    return this.update()
  }

  unbind (command, selector, act) {
    if (command in internal(this).builtinCommands &&
        internal(this).builtinCommands[command][selector] === act) {
      Reflect.deleteProperty(internal(this).builtinCommands[command], selector)
      if (Object.keys(internal(this).builtinCommands[command]) === 0) {
        Reflect.deleteProperty(internal(this).builtinCommands, command)
      }
    }
    return this.update()
  }

  get (selector, action) {
    const map = internal(this).profile.getKeymap()
    for (const sel in map) {
      const commands = map[sel]
      for (const command in commands) {
        const act = commands[command]
        if (sel === selector && act === action) {
          if (process.platform === 'darwin') {
            return command.replace('CmdOrCtrl', 'command')
          }
          return command.replace('CmdOrCtrl', 'ctrl')
        }
      }
    }

    for (const command in internal(this).commands) {
      const sels = internal(this).commands[command]
      for (const sel in sels) {
        const act = sels[sel]
        if (sel === selector && act === action) {
          if (process.platform === 'darwin') {
            return command.replace('CmdOrCtrl', 'command')
          }
          return command.replace('CmdOrCtrl', 'ctrl')
        }
      }
    }
    return null
  }

  update () {
    Mousetrap.reset()

    internal(this).commands = Object.assign({}, internal(this).builtinCommands)

    const map = internal(this).profile.getKeymap()
    for (const selector in map) {
      const commands = map[selector]
      for (const command in commands) {
        const act = commands[command]
        if (!(command in internal(this).commands)) {
          internal(this).commands[command] = {}
        }
        internal(this).commands[command][selector] = act
      }
    }

    for (let command in internal(this).commands) {
      const sels = internal(this).commands[command]
      if (process.platform === 'darwin') {
        command = command.replace('CmdOrCtrl', 'command')
      } else {
        command = command.replace('CmdOrCtrl', 'ctrl')
      }
      Mousetrap.bind(command, (event) => {
        for (const sel in sels) {
          const act = sels[sel]
          const active = !sel.startsWith('!') &&
            (event.target.matches(sel) ||
              event.target.parentNode.matches(sel).length)
          if (active) {
            if (typeof act === 'function') {
              if (act(event) === false) {
                event.preventDefault()
                event.stopPropagation()
              }
            } else {
              this.pubsub.emit(act)
              event.preventDefault()
              event.stopPropagation()
            }
          }
        }
      })
    }

    this.emit('update')
  }
}
