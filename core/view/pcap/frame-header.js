import m from 'mithril'

export default class FrameHeader {
  constructor () {
    this.offset = 0
    this.active = false
    this.labels = [
      { name: 'Protocol' },
      { name: 'Source' },
      { name: 'Destination' },
      { name: 'Length' },
      { name: 'Summary' }
    ]
    this.widthList =
      [].concat(deplug.workspace.get('_.pcap.table.widthList', []))
  }

  applyWidth () {
    this.active = false
    deplug.workspace.set('_.pcap.table.widthList', this.widthList)
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
        this.widthList[this.index] = width
      },
    })]
    let offset = 0
    for (let index = 0; index < this.labels.length; index += 1) {
      const label = this.labels[index]
      const last = index === this.labels.length - 1
      if (!(index in this.widthList)) {
        this.widthList[index] = 100
      }
      const width = this.widthList[index]
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
