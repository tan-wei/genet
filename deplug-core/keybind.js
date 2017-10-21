import { EventEmitter } from 'events'
import Mousetrap from 'mousetrap'

const fields = Symbol('fields')
export default class Keybind extends EventEmitter {
  constructor (profile) {
    super()
    this[fields] = {
      profile,
      builtinCommands: {},
      commands: {},
    }
  }

  bind (command, selector, act) {
    if (!(command in this[fields].builtinCommands)) {
      this[fields].builtinCommands[command] = {}
    }
    this[fields].builtinCommands[command][selector] = act
    return this.update()
  }

  unbind (command, selector, act) {
    if (command in this[fields].builtinCommands &&
        this[fields].builtinCommands[command][selector] === act) {
      Reflect.deleteProperty(this[fields].builtinCommands[command], selector)
      if (Object.keys(this[fields].builtinCommands[command]) === 0) {
        Reflect.deleteProperty(this[fields].builtinCommands, command)
      }
    }
    return this.update()
  }

  get (selector, action) {
    const map = this[fields].profile.getKeymap()
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

    for (const command in this[fields].commands) {
      const sels = this[fields].commands[command]
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

    this[fields].commands = Object.assign({}, this[fields].builtinCommands)

    const map = this[fields].profile.getKeymap()
    for (const selector in map) {
      const commands = map[selector]
      for (const command in commands) {
        const act = commands[command]
        if (!(command in this[fields].commands)) {
          this[fields].commands[command] = {}
        }
        this[fields].commands[command][selector] = act
      }
    }

    for (let command in this[fields].commands) {
      const sels = this[fields].commands[command]
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
