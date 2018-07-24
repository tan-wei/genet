import Dialog from '../../lib/dialog'
import FilterSuggest from './filter-suggest'
import FrameHeader from './frame-header'
import FrameListView from './frame-list-view'
import InputDialog from './input-dialog'
import OutputDialog from './output-dialog'
import ToolBar from './toolbar'
import m from 'mithril'
import path from 'path'
import { remote } from 'electron'
import tempy from 'tempy'
const { dialog } = remote
export default class TopView {
  constructor () {
    this.sess = null
    this.filtered = null
    this.displayFilter = ''
    this.suggestMaxCount = 6
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

  searchKeyPress (event) {
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

  view () {
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
          m('a', {
            onclick: () => {
              genet.action.global.emit('core:tab:reload')
            },
          }, [
            'Reload to apply changed configurations'
          ]),
          m('i', {
            class: 'fa fa-close',
            onclick: () => {
              this.showReloadBalloon = false
            },
          })
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

  oncreate () {
    genet.config.watch('', () => {
      if (!genet.config.get('_.package.noConfUpdated', false)) {
        this.showReloadBalloon = true
        m.redraw()
      }
    })
    genet.packages.once('updated', () => {
      genet.action.on('core:session:created', (sess) => {
        sess.on('update', () => m.redraw())
        this.viewState.capture = sess.status.stream
        this.sess = sess
        m.redraw()
      })
      if (genet.argv.import) {
        const file = path.resolve(genet.argv.import)
        genet.session.create().then((sess) => {
          for (const handler of genet.session.fileReaders) {
            if (handler(sess, { file }) === true) {
              break
            }
          }
          genet.action.emit('core:session:created', sess)
        })
      } else {
        const inputDialog = new Dialog(InputDialog)
        inputDialog.show({ cancelable: false })
      }
      m.redraw()
    })
    const filterInput = document.querySelector('input[name=display-filter]')
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
      this.sess.createWriter('pcap-file', { file: 'x.pcap' }, '$.index == 2')
      const outputDialog = new Dialog(OutputDialog,
        {
          displayFilter: this.displayFilter,
          checkedFrames: this.viewState.checkedFrames,
        })
      outputDialog.show({ cancelable: true }).then(async (filter) => {
        const file = dialog.showSaveDialog(
          { filters: genet.session.fileExtensions.exporter })
        if (typeof file !== 'undefined') {
          this.sess.createWriter(file, filter).then(() => {
            genet.notify.show(file, {
              type: 'sussess',
              title: 'Exported',
            })
          })
        }
      })
    })
    genet.action.global.on('core:pcap:focus-display-filter', () => {
      document.querySelector('input[name=display-filter]').focus()
    })
    genet.action.global.on('core:tab:reload', () => {
      let dump = Promise.resolve()
      if (this.sess) {
        const file = tempy.file({ extension: 'pcap' })
        genet.resumer.set('core:session:dump', file)
        dump = this.sess.exportFile(file, '')
      }
      dump.then(() => {
        genet.resumer.reload()
        genet.notify.show('Reloading...')
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
            [].concat(genet.workspace.get('_.filter.history', []))
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
