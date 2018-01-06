const m = require('mithril')
class WorkerView {
  constructor () {
    this.sess = null
    deplug.action.on('core:session:created', (sess) => {
      this.sess = sess
      m.redraw()
    })
  }

  view () {
    if (this.sess === null || this.sess.inspectorSessions === null) {
      return m('div', { class: 'worker-view' }, [
        'Debug sessions are disabled'
      ])
    }
    return m('div', { class: 'worker-view' }, [
      m('ul', this.sess.inspectorSessions.map((session) => m('li', [
          session.id
        ])))
    ])
  }
}

module.exports = WorkerView
