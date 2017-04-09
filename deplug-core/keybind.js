import { EventEmitter } from 'events'
import Mousetrap from 'mousetrap'
import jquery from 'jquery'

export default class Keybind extends EventEmitter {
  constructor (profile) {
    super()
    this.profile = profile
    this.builtinCommands = {}
    this.commands = {}
  }

  bind (command, selector, act) {
    if (!(command in this.builtinCommands)) {
      this.builtinCommands[command] = {}
    }
    this.builtinCommands[command][selector] = act
    return this.update()
  }

  unbind (command, selector, act) {
    if (command in this.builtinCommands &&
        this.builtinCommands[command][selector] === act) {
      Reflect.deleteProperty(this.builtinCommands[command], selector)
      if (Object.keys(this.builtinCommands[command]) === 0) {
        Reflect.deleteProperty(this.builtinCommands, command)
      }
    }
    return this.update()
  }

  get (selector, action) {
    const map = this.profile.getKeymap()
    for (const sel in map) {
      const commands = map[sel]
      for (const command in commands) {
        const act = commands[command]
        if (sel === selector && act === action) {
          if (process.platform !== 'darwin') {
            return command.replace('command', 'ctrl')
          }
            return command

        }
      }
    }

    for (const command in this.commands) {
      const sels = this.commands[command]
      for (const sel in sels) {
        const act = sels[sel]
        if (sel === selector && act === action) {
          if (process.platform !== 'darwin') {
            return command.replace('command', 'ctrl')
          }
          return command
        }
      }
    }
    return null
  }

  update () {
    Mousetrap.reset()

    this.commands = Object.assign({}, this.builtinCommands)

    const map = this.profile.getKeymap()
    for (const selector in map) {
      const commands = map[selector]
      for (const command in commands) {
        const act = commands[command]
        if (!(command in this.commands)) {
          this.commands[command] = {}
        }
        this.commands[command][selector] = act
      }
    }

    for (let command in this.commands) {
      const sels = this.commands[command]
      if (process.platform !== 'darwin') {
        command = command.replace('command', 'ctrl')
      }
      Mousetrap.bind(command, (event) => {
        for (const sel in sels) {
          const act = sels[sel]
          const active = !sel.startsWith('!') &&
            (jquery(event.target).is(sel) ||
              jquery(event.target).parents(sel).length)
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
