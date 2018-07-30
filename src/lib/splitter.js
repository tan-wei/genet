import genet from '@genet/api'
import m from 'mithril'

const fields = Symbol('fields')
export class HSplitter {
  constructor () {
    this[fields] = {
      width: 320,
      handle: 0,
      active: false,
    }
  }

  oninit (vnode) {
    const { width, workspace } = vnode.attrs
    if (Number.isInteger(width)) {
      this[fields].width = width
    }
    if (typeof workspace === 'string') {
      this[fields].width = genet.workspace.get(workspace, this[fields].width)
    }
    this[fields].handle = this[fields].width
  }

  view (vnode) {
    const { width, handle, active } = this[fields]
    const { left, right, workspace } = vnode.attrs
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
          this.applyWidth(workspace)
        },
        onmouseout: () => {
          this.applyWidth(workspace)
        },
        onmousemove: (event) => {
          this[fields].handle = event.offsetX
        },
      })
    ])
  }

  applyWidth (workspace) {
    this[fields].width = this[fields].handle
    this[fields].active = false
    if (typeof workspace === 'string') {
      genet.workspace.set(workspace, this[fields].width)
    }
  }
}

export class VSplitter {
  constructor () {
    this[fields] = {
      height: 320,
      handle: 0,
      active: false,
    }
  }

  oninit (vnode) {
    const { height, workspace } = vnode.attrs
    if (Number.isInteger(height)) {
      this[fields].height = height
    }
    if (typeof workspace === 'string') {
      this[fields].height = genet.workspace.get(workspace, this[fields].height)
    }
    this[fields].handle = this[fields].height
  }

  view (vnode) {
    const { height, handle, active } = this[fields]
    const { bottom, top, workspace } = vnode.attrs
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
          this.applyHeight(workspace)
        },
        onmouseout: () => {
          this.applyHeight(workspace)
        },
        onmousemove: (event) => {
          this[fields].handle = event.target.clientHeight - event.offsetY
        },
      })
    ])
  }

  applyHeight (workspace) {
    this[fields].height = this[fields].handle
    this[fields].active = false
    if (typeof workspace === 'string') {
      genet.workspace.set(workspace, this[fields].height)
    }
  }
}
