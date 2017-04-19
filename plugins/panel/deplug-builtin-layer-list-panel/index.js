import m from 'mithril'
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
    return <div class="layer-list-view">
      { this.frame.primaryLayer.summary }
    </div>
  }
}
