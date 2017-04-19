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
    const tsString = ts.format('YYYY-MM-DDTHH:mm:ss.SSS') +
      this.frame.timestamp.nsec + ts.format('Z')

    let length = `${this.frame.rootLayer.payload.length}`
    if (this.frame.length > this.frame.rootLayer.payload.length) {
      length += ` (actual: ${this.frame.length})`
    }
    return <div class="layer-list-view">
      <table>
        <tr><td>Timestamp </td><td>{ tsString }</td></tr>
        <tr><td>Length </td><td>{ length }</td></tr>
      </table>
    </div>
  }
}
