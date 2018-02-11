import Fuse from 'fuse.js'
import m from 'mithril'

export default class FilterSuggest {
  view (vnode) {
    const { enabled, hint } = vnode.attrs

    let items = []
    if (enabled) {
      const source = Array.from(deplug.session.tokens.entries())
        .map(([id, item]) => ({
          id,
          item,
        }))
      const fuse = new Fuse(source, { keys: ['id', 'item.name'] })
      items = fuse.search(hint).slice(0, 6)
    }

    return m('div', {
      class: 'suggest',
      style: {
        display: items.length
          ? 'block'
          : 'none',
      },
    }, [
      m('ul', items.map(({ id, item }) => m('li', [
        id,
        m('span', { class: 'description' }, [item.name])
      ])))
    ])
  }
}
