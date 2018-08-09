import m from 'mithril'

export default class FilterError {
  view(vnode) {
    return m('div', {
      class: 'error',
      style: {
        display: vnode.attrs.error
          ? 'block'
          : 'none',
      },
    }, [
        m('p', [vnode.attrs.error])
      ])
  }
}
