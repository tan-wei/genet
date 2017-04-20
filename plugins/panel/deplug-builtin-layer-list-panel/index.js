import m from 'mithril'
import moment from 'moment'
import { Channel } from 'deplug'

class PropertyItem {
  view(vnode) {
    const prop = vnode.attrs.property
    return <tr>
      <td>{ prop.name }</td>
      <td>{ prop.summary || prop.value.toString() }</td>
      <td>{ prop.value.toString() }</td>
    </tr>
  }
}

class LayerItem {
  view(vnode) {
    const layer = vnode.attrs.layer
    return <ul>
      <li>{ layer.name }</li>
      <table>
        {
          layer.properties.map((prop) => {
            return m(PropertyItem, {property: prop})
          })
        }
      </table>
      <ul>
        {
          layer.children.map((child) => {
            return m(LayerItem, {layer: child})
          })
        }
      </ul>
    </ul>
  }
}

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
        { m(LayerItem, {layer: this.frame.rootLayer}) }
      </table>
    </div>
  }
}
