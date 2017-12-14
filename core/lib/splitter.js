import m from 'mithril'

const fields = Symbol('fields')
export class HSplitter {
  constructor () {
    this[fields] = {
      width: 180,
      active: false,
    }
  }

  view (vnode) {
    const { width, active } = this[fields]
    const { left, right } = vnode.attrs
    return m('div', { class: 'splitter' }, [
      m('div', {
        class: 'left',
        style: { width: `${width}px` },
      }, [
        m(left, vnode.attrs)
      ]),
      m('div', {
        class: 'right',
        style: { left: `${width}px` },
       }, [
        m(right, vnode.attrs)
      ]),
      m('div', {
        class: 'handle',
        style: { left: `${width}px` },
        active,
        onmousedown: (event) => {
          this[fields].active = true
        },
        onmouseup: () => {
          this[fields].active = false
        },
      }),
      m('div', {
        class: 'base',
        style: {
          display: active
            ? 'block'
            : 'none',
        },
        onmouseup: () => {
          this[fields].active = false
        },
        onmouseout: () => {
          this[fields].active = false
        },
        onmousemove: (event) => {
          this[fields].width = event.offsetX
        },
      })
    ])
  }
}
