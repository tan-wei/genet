const { remote } = require('electron')
const { BrowserWindow } = remote
const m = require('mithril')
class SessionView {
  view (vnode) {
    const { session } = vnode.attrs
    return m('li', [
      m('h4', [session.id]),
      m('input', {
        type: 'button',
        value: 'Inspect...',
        onclick: () => {
          const win = new BrowserWindow()
          win.loadURL(session.devtoolsFrontendUrl)
        },
      })
    ])
  }
}

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
      return m('div', { class: 'view' }, [
        m('p', ['Debug sessions are disabled'])
      ])
    }
    return m('div', { class: 'view' }, [
      m('ul', this.sess.inspectorSessions.map((session) =>
        m(SessionView, { session })))
    ])
  }
}

module.exports = WorkerView
