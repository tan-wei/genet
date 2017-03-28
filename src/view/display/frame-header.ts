import genet from '@genet/api'
import m from 'mithril'

export default class FrameHeader {
  private index: number
  private offset: number
  private active: boolean
  private labels: any[]
  private viewState: any
  constructor() {
    this.offset = 0
    this.active = false
    this.labels = [
      { name: 'Index' },
      { name: 'Protocol' }
    ]
    const columns =
      genet.config.get('_.framelist.columns', [])
    for (const col of columns) {
      this.labels.push(col)
    }
    this.labels.push({ name: 'Summary' })
  }

  applyWidth() {
    this.active = false
  }

  oninit(vnode) {
    this.viewState = vnode.attrs.viewState
  }

  view() {
    const views = [m('div', {
      class: 'base',
      style: {
        display: this.active
          ? 'block'
          : 'none',
      },
      onmouseup: () => {
        this.applyWidth()
      },
      onmouseout: () => {
        this.applyWidth()
      },
      onmousemove: (event) => {
        const width = Math.max(10, event.offsetX - this.offset)
        this.viewState.headerWidthList[this.index] = width
      },
    })]
    let offset = 0
    for (let index = 0; index < this.labels.length; index += 1) {
      const label = this.labels[index]
      const last = index === this.labels.length - 1
      if (!(index in this.viewState.headerWidthList)) {
        this.viewState.headerWidthList[index] = 100
      }
      const width = this.viewState.headerWidthList[index]
      offset += width
      if (!last) {
        views.push(m('div', {
          class: 'handle',
          'data-offset': offset - width,
          style: {
            left: `${offset}px`,
            display: this.active
              ? 'none'
              : 'block',
          },
          onmousedown: (event) => {
            this.index = index
            this.active = true
            this.offset =
              Number.parseInt(event.target.getAttribute('data-offset'), 10)
          },
          onmouseup: () => {
            this.active = false
          },
        }))
      }
      views.push(m('span',
        {
          style: {
            width: last
              ? 'auto'
              : `${width}px`,
          },
        }, [
          label.name
        ]))
    }
    return m('div', { class: 'frame-header' }, views)
  }
}
