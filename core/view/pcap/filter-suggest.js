import Fuse from 'fuse.js'
import debounce from 'lodash.debounce'
import m from 'mithril'

export default class FilterSuggest {
  constructor () {
    this.items = []
    this.hint = ''
    this.update = debounce(() => {
      const source = Array.from(deplug.session.tokens.entries())
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
  view (vnode) {
    const { enabled, hint } = vnode.attrs
    if (this.hint !== hint) {
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
      m('ul', this.items.map(({ id, item }) => m('li', [
        id,
        m('span', { class: 'description' }, [item.name])
      ])))
    ])
  }
}
