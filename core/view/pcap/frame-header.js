import m from 'mithril'

export default class FrameHeader {
  constructor () {
    this.offset = 0
    this.active = false
    this.labels = [
      {
        name: 'Protocol',
        width: 100,
      },
      {
        name: 'Source',
        width: 100,
      },
      {
        name: 'Destination',
        width: 100,
      },
      {
        name: 'Length',
        width: 100,
      },
      {
        name: 'Summary',
        width: 100,
      }
    ]
  }

  applyWidth () {
    this.active = false
  }

  view () {
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
        this.labels[this.index].width = width
      },
    })]
    let offset = 0
    for (let index = 0; index < this.labels.length; index += 1) {
      const label = this.labels[index]
      const last = index === this.labels.length - 1
      offset += label.width
      if (!last) {
        views.push(m('div', {
          class: 'handle',
          'data-offset': offset - label.width,
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
              : `${label.width}px`,
          },
        }, [
          label.name
        ]))
    }
    return m('div', { class: 'frame-header' }, views)
  }
}
