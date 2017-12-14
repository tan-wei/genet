import m from 'mithril'

const fields = Symbol('fields')
export class VSplitter {
  constructor () {
    this[fields] = {
      width: 320,
      active: false,
    }
  }

  oninit (vnode) {
    const { width } = vnode.attrs
    if (Number.isInteger(width)) {
      this[fields].width = width
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
        class: 'handle vertical',
        style: { left: `${width}px` },
        active,
        onmousedown: () => {
          this[fields].active = true
        },
        onmouseup: () => {
          this[fields].active = false
        },
      }),
      m('div', {
        class: 'base vertical',
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

export class HSplitter {
  constructor () {
    this[fields] = {
      height: 320,
      active: false,
    }
  }

  oninit (vnode) {
    const { height } = vnode.attrs
    if (Number.isInteger(height)) {
      this[fields].height = height
    }
  }

  view (vnode) {
    const { height, active } = this[fields]
    const { bottom, top } = vnode.attrs
    return m('div', { class: 'splitter vertical' }, [
      m('div', {
        class: 'bottom',
        style: { height: `${height}px` },
      }, [
        m(bottom, vnode.attrs)
      ]),
      m('div', {
        class: 'top',
        style: { bottom: `${height}px` },
       }, [
        m(top, vnode.attrs)
      ]),
      m('div', {
        class: 'handle horizontal',
        style: { bottom: `${height}px` },
        active,
        onmousedown: () => {
          this[fields].active = true
        },
        onmouseup: () => {
          this[fields].active = false
        },
      }),
      m('div', {
        class: 'base horizontal',
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
          this[fields].height = event.target.clientHeight - event.offsetY
        },
      })
    ])
  }
}
