import m from 'mithril'

export default class PcapDetailView {
  constructor () {
    this.selectedFrame = null
    deplug.action.on('core:frame:selected', (frame) => {
      this.selectedFrame = frame
      m.redraw()
    })
  }

  view () {
    return [
      m('h1', ['Deplug']),
      m('p', [
        (this.selectedFrame
          ? this.selectedFrame.length
          : '')
      ])
    ]
  }
}
