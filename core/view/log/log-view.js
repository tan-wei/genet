import m from 'mithril'

export default class LogView {
  constructor () {
    deplug.config.watch('_.maxLogLength', (value) => {
      this.maxLength = value
    }, 1000)
  }
  view () {
    return [m('p'), [this.maxLength]]
  }
}
