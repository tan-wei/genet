import m from 'mithril'

export default class FilterSuggest {
  view (vnode) {
    const { enabled } = vnode.attrs

    const items = Array.from(deplug.session.tokens.entries()).slice(0, 5)
    return m('div', {
      class: 'suggest',
      style: {
        display: enabled
          ? 'block'
          : 'none',
      },
    }, [
      m('ul', items.map(([key, item]) => m('li', [
        key,
        m('span', { class: 'description' }, [item.name])
      ])))
    ])
  }
}
