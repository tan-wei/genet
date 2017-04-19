import m from 'mithril'
import moment from 'moment'
import { Channel } from 'deplug'

export default class LayerListView {
  constructor() {
    this.frame = null
    Channel.on('core:frame:selected', (frame) => {
      this.frame = frame
      m.redraw()
    })
  }

  view(vnode) {
    if (!this.frame) {
      return <div class="layer-list-view">No frames selected</div>
    }
    const ts = moment(this.frame.timestamp)
    return <div class="layer-list-view">
      <p>{ this.frame.primaryLayer.summary }</p>
      <p>{ ts.format('YYYY-MM-DDTHH:mm:ss.SSS') + this.frame.timestamp.nsec + ts.format('Z') }</p>
    </div>
  }
}
