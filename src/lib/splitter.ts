import genet from '@genet/api'
import m from 'mithril'

export class HSplitter {
  private _width: number
  private _handle: number
  private _active: boolean
  constructor() {
    this._width = 320
    this._handle = 0
    this._active = false
  }

  oninit(vnode) {
    const { width, workspace } = vnode.attrs
    if (Number.isInteger(width)) {
      this._width = width
    }
    if (typeof workspace === 'string') {
      this._width = genet.workspace.get(workspace, this._width)
    }
    this._handle = this._width
  }

  view(vnode) {
    const { left, right, workspace } = vnode.attrs
    return m('div', { class: 'splitter' }, [
      m('div', {
        class: 'left',
        style: { width: `${this._width}px` },
      }, [
          m(left, vnode.attrs)
        ]),
      m('div', {
        class: 'right',
        style: { left: `${this._width}px` },
      }, [
          m(right, vnode.attrs)
        ]),
      m('div', {
        class: 'handle vertical',
        style: { left: `${this._handle}px` },
        active: this._active,
        onmousedown: () => {
          this._handle = this._width
          this._active = true
        },
        onmouseup: () => {
          this._active = false
        },
      }),
      m('div', {
        class: 'base vertical',
        style: {
          display: this._active
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
          this._handle = event.offsetX
        },
      })
    ])
  }

  applyWidth(workspace: string) {
    this._width = this._handle
    this._active = false
    genet.workspace.set(workspace, this._width)
  }
}

export class VSplitter {
  private _height: number
  private _handle: number
  private _active: boolean
  constructor() {
    this._height = 320
    this._handle = 0
    this._active = false
  }

  oninit(vnode) {
    const { height, workspace } = vnode.attrs
    if (Number.isInteger(height)) {
      this._height = height
    }
    if (typeof workspace === 'string') {
      this._height = genet.workspace.get(workspace, this._height)
    }
    this._handle = this._height
  }

  view(vnode) {
    const { bottom, top, workspace } = vnode.attrs
    return m('div', { class: 'splitter vertical' }, [
      m('div', {
        class: 'bottom',
        style: { height: `${this._height}px` },
      }, [
          m(bottom, vnode.attrs)
        ]),
      m('div', {
        class: 'top',
        style: { bottom: `${this._height}px` },
      }, [
          m(top, vnode.attrs)
        ]),
      m('div', {
        class: 'handle horizontal',
        style: { bottom: `${this._handle}px` },
        active: this._active,
        onmousedown: () => {
          this._active = true
        },
        onmouseup: () => {
          this._active = false
        },
      }),
      m('div', {
        class: 'base horizontal',
        style: {
          display: this._active
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
          this._handle = event.target.clientHeight - event.offsetY
        },
      })
    ])
  }

  applyHeight(workspace: string) {
    this._height = this._handle
    this._active = false
    genet.workspace.set(workspace, this._height)
  }
}
