import Dialog from '../../lib/dialog'
import FilterSuggest from './filter-suggest'
import FrameHeader from './frame-header'
import FrameListView from './frame-list-view'
import InputDialog from './input-dialog'
import OutputDialog from './output-dialog'
import ToolBar from './toolbar'
import genet from '@genet/api'
import m from 'mithril'
import path from 'path'
import tempy from 'tempy'
export default class TopView {
  private sess: any
  private displayFilter: string
  private suggestMaxCount: number
  private suggestEnabled: boolean
  private suggestHint: string
  private suggestIndex: number
  private showReloadBalloon: boolean
  private viewState: any
  constructor() {
    this.sess = null
    this.displayFilter = ''
    this.suggestEnabled = false
    this.suggestHint = ''
    this.suggestIndex = -1
    this.showReloadBalloon = false
    this.viewState = {
      capture: false,
      scrollLock: false,
      selectedFrame: -1,
      checkedFrames: new Set(),
      headerWidthList: [],
    }
  }

  searchKeyPress(event) {
    switch (event.code) {
      case 'Enter':
        {
          this.suggestIndex = -1
          this.suggestEnabled = false
          const filter = event.target.value.trim()
          genet.action.emit('core:filter:set', filter)
          genet.resumer.set('core:filter', filter)
        }
        break
      case 'ArrowDown':
        if (this.suggestEnabled) {
          genet.action.emit('core:filter:suggest:next')
        }
        break
      case 'ArrowUp':
        if (this.suggestEnabled) {
          event.preventDefault()
          genet.action.emit('core:filter:suggest:prev')
        }
        break
      default:
        this.suggestEnabled = true
    }
  }

  view() {
    this.viewState.counter = '0'
    if (this.sess) {
      const { status } = this.sess
      this.viewState.counter = status.filters.main
        ? `${status.filters.main.frames} / ${status.frames}`
        : `${status.frames}`
    }
    return [
      m('header', [
        m('div', {
          class: 'balloon',
          style: {
            display: this.showReloadBalloon
              ? 'block'
              : 'none',
          },
        }, [
            m('span', ['Configuration has been changed']),
            m('a', {
              onclick: () => {
                genet.action.global.emit('core:tab:reload')
              },
            }, [
                'Reload'
              ]),
            m('a', {
              onclick: () => {
                this.showReloadBalloon = false
              },
            }, ['Dismiss'])
          ]),
        m('input', {
          type: 'text',
          placeholder: 'Display Filter',
          onkeydown: (event) => {
            this.searchKeyPress(event)
          },
          onkeyup: (event) => {
            this.suggestHint = event.target.value
          },
          onfocus: () => {
            this.suggestEnabled = true
          },
          onblur: () => {
            this.suggestEnabled = false
          },
          name: 'display-filter',
        }),
        m(FilterSuggest, {
          enabled: this.suggestEnabled,
          hint: this.suggestHint,
        }),
        m(ToolBar, {
          viewState: this.viewState,
          sess: this.sess,
        }),
        m(FrameHeader, { viewState: this.viewState })
      ]),
      this.sess
        ? m(FrameListView, {
          sess: this.sess,
          viewState: this.viewState,
        })
        : m('nav')
    ]
  }

  oncreate() {
    genet.config.watch('', () => {
      this.showReloadBalloon = true
      m.redraw()
    })
    genet.packages.once('updated', () => {
      genet.action.on('core:session:created', (sess) => {
        sess.on('update', () => m.redraw())
        sess.on('error', (err) => {
          genet.notify.show(err, {
            type: 'error',
            title: 'Session Error',
          })
        })
        this.viewState.capture = sess.status.stream
        this.sess = sess
        if (genet.resumer.has('core:filter')) {
          genet.action.emit('core:filter:set', genet.resumer.get('core:filter'))
        }
        if (genet.resumer.has('core:session:dump')) {
          const file = genet.resumer.get('core:session:dump')
          for (const { handler } of genet.session.fileReaders) {
            if (handler(sess, { file }) === true) {
              break
            }
          }
        }
        if (genet.resumer.has('core:session:stream-reader')) {
          const { name, stream } = genet.resumer.get('core:session:stream-reader')
          sess.regiterStreamReader(name, stream)
          sess.startStream()
        }
        m.redraw()
      })
      if (genet.argv.import) {
        const file = path.resolve(genet.argv.import)
        genet.session.create().then((sess) => {
          for (const { handler } of genet.session.fileReaders) {
            if (handler(sess, { file }) === true) {
              break
            }
          }
          genet.action.emit('core:session:created', sess)
        })
      } else if (genet.resumer.has('core:session:stream-reader')) {
        genet.session.create().then((sess) => {
          genet.action.emit('core:session:created', sess)
        })
      } else {
        const inputDialog = new Dialog(InputDialog)
        inputDialog.show({ cancelable: false })
      }
      m.redraw()
    })
    const filterInput = document.querySelector('input[name=display-filter]') as HTMLInputElement
    genet.action.on('core:filter:suggest:hint-selected', (hint, enter) => {
      filterInput.value = hint
      filterInput.selectionStart = filterInput.value.length
      if (enter) {
        this.suggestIndex = -1
        this.suggestEnabled = false
        genet.action.emit('core:filter:set', hint.trim())
      }
    })
    genet.action.global.on('core:file:export', () => {
      const outputDialog = new Dialog(OutputDialog,
        {
          sess: this.sess,
          displayFilter: this.displayFilter,
          checkedFrames: this.viewState.checkedFrames,
        })
      outputDialog.show({ cancelable: true })
    })
    genet.action.global.on('core:display:focus-filter', () => {
      const input = document.querySelector('input[name=display-filter]')
      if (input instanceof HTMLInputElement) {
        input.focus()
      }
    })
    genet.action.global.on('core:tab:reload', () => {
      let dump = Promise.resolve()
      if (this.sess) {
        const file = tempy.file({ extension: 'pcap' })
        genet.resumer.set('core:session:dump', file)
        dump = this.sess.createWriter('pcap-file', { file })
      }
      dump.then(() => {
        genet.resumer.reload()
        genet.notify.show('Reloading...')
      }).catch((err) => {
        genet.notify.show(
          err.message, {
            type: 'error',
            title: 'Dump Error',
          })
      })
    })
    genet.action.on('core:filter:set', (value) => {
      try {
        filterInput.value = value
        this.displayFilter = value
        this.sess.setFilter('main', value)
        if (value.length > 0) {
          const maxLength = 10
          const history =
            [].concat(genet.workspace.get('_.filter.history', [])) as string[]
          history.push(value)
          const overflow = history.length - maxLength
          if (overflow > 0) {
            history.splice(0, overflow)
          }
          genet.workspace.set('_.filter.history', history)
        }
        const filter = value.length > 0
          ? genet.session.createFilterCompiler()
            .compile(value)
          : null
        genet.action.emit('core:filter:updated', filter)
      } catch (err) {
        genet.notify.show(
          err.message, {
            type: 'error',
            title: 'Filter Error',
          })
      }
    })
  }
}
