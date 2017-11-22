import { ipcRenderer } from 'electron'
import m from 'mithril'
import moment from 'moment'

export default class LogView {
  constructor () {
    this.logs = []
    deplug.config.watch('_.maxLogLength', (value) => {
      this.maxLength = value
    }, 1000)
    ipcRenderer.on('core:logger:message', (event, message) => {
      this.logs.push(message)
      const overflow = this.logs.length - this.maxLength
      if (overflow > 0) {
        this.logs.splice(0, overflow)
      }
      m.redraw()
    })
  }
  view () {
    return m('main', [
      m('ul', this.logs.map((line) => {
        const timestamp = moment(line.timestamp)
        return m('li', { class: line.level }, [
          m('span', {
            class: 'timestamp',
            'data-balloon': timestamp.fromNow(),
            'data-balloon-pos': 'down',
          }, [timestamp.format()]),
          m('details', [
            m('summary', [
              line.message
            ]),
            line.message
          ])
        ])
      }))
    ])
  }
}
