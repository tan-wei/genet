import m from 'mithril'

export default class FrameHeader {
  view () {
    const labels = [
      {
        name: 'Protocol',
        width: 100,
      },
      {
        name: 'Length',
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
        name: 'Summary',
        width: 100,
      }
    ]
    return m('div', { class: 'frame-header' },
      labels.map((item) => m('span', { style: { width: `${item.width}px` } }, [
        item.name,
        m('div', { class: 'handle' })
      ])))
  }
}
