import { HSplitter, VSplitter } from '../../lib/splitter'
import Dialog from '../../lib/dialog'
import ExportDialog from './export-dialog'
import FrameListView from './frame-list-view'
import PcapDetailView from './detail'
import PcapDialog from './pcap-dialog'
import PcapToolView from './tool'
import m from 'mithril'
import path from 'path'
import { remote } from 'electron'
const { dialog } = remote
class BottomView {
  constructor () {
    this.sess = null
    this.capture = false
    this.filtered = null
    this.scrollLock = false
    this.displayFilter = ''
  }

  searchKeyPress (event) {
    switch (event.code) {
      case 'Enter':
        deplug.action.emit('core:filter:set', event.target.value.trim())
        break
      default:
    }
  }

  view () {
    let counter = '0'
    if (this.sess) {
      counter = this.sess.filter.main
        ? `${this.sess.filter.main.frames} / ${this.sess.frame.frames}`
        : `${this.sess.frame.frames}`
    }
    return [
      m('header', [
        m('input', {
          type: 'text',
          placeholder: 'Display Filter',
          onkeydown: (event) => {
            this.searchKeyPress(event)
          },
          name: 'display-filter',
        }),
        m('span', {
          class: 'button',
          'data-balloon': `Capture ${this.capture
            ? 'Running'
            : 'Paused'}`,
          'data-balloon-pos': 'right',
          onclick: () => {
            const { sess } = this
            if (this.capture) {
              sess.stopPcap()
            } else {
              sess.startPcap()
            }
            this.capture = !this.capture
          },
        }, [
          m('i', {
            class: this.capture
              ? 'fa fa-play-circle'
              : 'fa fa-pause-circle',
          })
        ]),
        m('span', {
          class: 'button',
          'data-balloon': `Scroll ${this.scrollLock
            ? 'Locked'
            : 'Unlocked'}`,
          'data-balloon-pos': 'right',
          onclick: () => {
            this.scrollLock = !this.scrollLock
          },
        }, [
          m('i', {
            class: this.scrollLock
              ? 'fa fa-lock'
              : 'fa fa-unlock-alt',
          })
        ]),
        m('span', {
          'data-balloon': 'Frame Counter',
          'data-balloon-pos': 'right',
        }, [counter])
      ]),
      this.sess
        ? m(FrameListView, {
          sess: this.sess,
          scrollLock: this.scrollLock,
        })
        : m('nav')
    ]
  }

  oncreate () {
    if (deplug.argv.import) {
      const file = path.resolve(deplug.argv.import)
      const browserWindow = remote.getCurrentWindow()
      deplug.packages.once('updated', () => {
        deplug.session.create().then((sess) => {
          deplug.action.emit('core:session:created', sess)
          this.sess = sess
          sess.on('frame', () => {
            m.redraw()
          })
          sess.on('status', (stat) => {
            const progress = stat.importerProgress >= 1.0
              ? -1
              : stat.importerProgress
            browserWindow.setProgressBar(progress)
            this.capture = stat.capture
            m.redraw()
          })
          sess.on('filter', () => {
            m.redraw()
          })
          sess.importFile(file)
        })
      })
    } else {
      const pcapDialog = new Dialog(PcapDialog)
      pcapDialog.show({ cancelable: false }).then(async (ifs) => {
        const sess = await deplug.session.create(ifs)
        deplug.action.emit('core:session:created', sess)
        this.sess = sess
        sess.startPcap()
        sess.on('frame', () => {
          m.redraw()
        })
        sess.on('status', (stat) => {
          this.capture = stat.capture
          m.redraw()
        })
        sess.on('filter', () => {
          m.redraw()
        })
      })
    }
    const filterInput = document.querySelector('input[name=display-filter]')
    deplug.action.global.on('core:file:export', () => {
      const exportDialog = new Dialog(ExportDialog,
        { displayFilter: this.displayFilter })
      exportDialog.show({ cancelable: true }).then(async (filter) => {
        const file = dialog.showSaveDialog(
          { filters: deplug.session.fileExtensions.exporter })
        if (typeof file !== 'undefined') {
          this.sess.exportFile(file, filter)
        }
      })
    })
    deplug.action.global.on('core:pcap:focus-display-filter', () => {
      document.querySelector('input[name=display-filter]').focus()
    })
    deplug.action.on('core:filter:set', (value) => {
      try {
        filterInput.value = value
        this.displayFilter = value
        this.sess.setDisplayFilter('main', value)
        if (value.length > 0) {
          const maxLength = deplug.config.get('_.filter.historyLength', 10)
          const history =
            [].concat(deplug.workspace.get('_.filter.history', []))
          history.push(value)
          const overflow = history.length - maxLength
          if (overflow > 0) {
            history.splice(0, overflow)
          }
          deplug.workspace.set('_.filter.history', history)
        }
        const filter = value.length > 0
          ? this.sess.createFilter(value)
          : null
        deplug.action.emit('core:filter:updated', filter)
      } catch (err) {
        deplug.notify.show(
          err.message, {
            type: 'error',
            title: 'Filter Error',
          })
      }
    })
  }
}

class MainView {
  view () {
    return m('main', [
      m(VSplitter, {
        right: PcapDetailView,
        left: PcapToolView,
        parent: this,
        width: 350,
        workspace: '_.pcapToolWidth',
      })
    ])
  }
}

export default class PcapView {
  view () {
    return [
      m(HSplitter, {
        top: BottomView,
        bottom: MainView,
        parent: this,
        height: 300,
        workspace: '_.pcapBottomHeight',
      }),
      m('div', { class: 'notification' })
    ]
  }
}
