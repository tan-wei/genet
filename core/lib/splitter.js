import m from 'mithril'

const fields = Symbol('fields')
export class VSplitter {
  constructor () {
    this[fields] = {
      width: 320,
      handle: 0,
      active: false,
    }
  }

  oninit (vnode) {
    const { width } = vnode.attrs
    if (Number.isInteger(width)) {
      this[fields].width = width
    }
    this[fields].handle = this[fields].width
  }

  view (vnode) {
    const { width, handle, active } = this[fields]
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
        style: { left: `${handle}px` },
        active,
        onmousedown: () => {
          this[fields].handle = this[fields].width
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
          this[fields].width = this[fields].handle
          this[fields].active = false
        },
        onmouseout: () => {
          this[fields].width = this[fields].handle
          this[fields].active = false
        },
        onmousemove: () => {
          this[fields].handle = event.offsetX
        },
      })
    ])
  }
}

export class HSplitter {
  constructor () {
    this[fields] = {
      height: 320,
      handle: 0,
      active: false,
    }
  }

  oninit (vnode) {
    const { height } = vnode.attrs
    if (Number.isInteger(height)) {
      this[fields].height = height
    }
    this[fields].handle = this[fields].height
  }

  view (vnode) {
    const { height, handle, active } = this[fields]
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
        style: { bottom: `${handle}px` },
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
          this[fields].height = this[fields].handle
          this[fields].active = false
        },
        onmouseout: () => {
          this[fields].height = this[fields].handle
          this[fields].active = false
        },
        onmousemove: () => {
          this[fields].handle = event.target.clientHeight - event.offsetY
        },
      })
    ])
  }
}
