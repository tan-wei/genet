import Fuse from 'fuse.js'
import debounce from 'lodash.debounce'
import m from 'mithril'

export default class FilterSuggest {
  constructor () {
    this.items = []
    this.hint = ''
    this.index = -1
    this.locked = 0
    this.update = debounce(() => {
      const source = Array.from(deplug.session.tokens.entries())
        .filter(([id]) => (/^[^!@[]/).test(id))
        .map(([id, item]) => ({
          id,
          item,
        }))
        .concat(deplug.workspace.get('_.filter.history', []).map((history) => ({
          id: history,
          item: { name: '(history)' },
        })))
      const fuse = new Fuse(source, { keys: ['id', 'item.name'] })
      this.items = fuse.search(this.hint).slice(0, 6)
      m.redraw()
    }, 200)
  }

  oncreate () {
    deplug.action.on('core:filter:suggest:next', () => {
      this.locked = 1
      if (this.index < 0) {
        this.locked += 1
      }
      if (this.items.length > 0) {
        this.index = (this.index + 1) % this.items.length
        this.updateCursor()
      }
    })
    deplug.action.on('core:filter:suggest:prev', () => {
      if (this.items.length > 0) {
        this.locked = 1
        if (this.index < 0) {
          this.index = this.items.length
          this.locked += 1
        }
        this.index = (this.items.length + this.index - 1) % this.items.length
        this.updateCursor()
      }
    })
  }

  updateCursor (enter = false) {
    this.locked = 2
    this.hint = this.items[this.index].id
    deplug.action.emit('core:filter:suggest:hint-selected', this.hint, enter)
  }

  view (vnode) {
    const { enabled, hint } = vnode.attrs
    if (this.items.length === 0 ||
        !this.items.some((item) => item.id === hint)) {
      if (this.locked > 0) {
        this.locked -= 1
      }
    }
    if (hint === '') {
      this.locked = 0
    }
    if (this.hint !== hint && this.locked === 0) {
      this.index = -1
      this.hint = hint
      this.update()
    }
    return m('div', {
      class: 'suggest',
      style: {
        display: enabled && this.items.length
          ? 'block'
          : 'none',
      },
    }, [
      m('ul', this.items.map(({ id, item }, index) => m('li',
        {
          active: index === this.index,
          onmousedown: (event) => {
            this.index = index
            this.updateCursor(true)
            event.preventDefault()
          },
        }, [
          id,
          m('span', { class: 'description' }, [item.name])
        ])))
    ])
  }
}
