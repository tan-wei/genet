import m from 'mithril'
import moment from 'moment'
import { Channel } from 'deplug'

class PropertyItem {
  view(vnode) {
    const prop = vnode.attrs.property
    const value = (prop.value == null ? '' : prop.value.toString())
    return <tr>
      <td>{ prop.name }</td>
      <td>{ prop.summary || value }</td>
      <td>{ value }</td>
      <td>{ prop.range }</td>
      <td>{ prop.error }</td>
      <td>
      <table>
        {
          prop.properties.map((prop) => {
            return m(PropertyItem, {property: prop})
          })
        }
      </table>
      </td>
    </tr>
  }
}

class LayerItem {
  view(vnode) {
    const layer = vnode.attrs.layer
    return <ul>
      <li>{ layer.name } ( { layer.namespace } )</li>
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
    this.frames = []
    Channel.on('core:frame:selected', (frames) => {
      this.frames = frames
      m.redraw()
    })
  }

  view(vnode) {
    if (!this.frames.length) {
      return <div class="layer-list-view">No frames selected</div>
    }
    return <div>
      {
        this.frames.map((frame) => {
          const ts = moment(frame.timestamp)
          const tsString = ts.format('YYYY-MM-DDTHH:mm:ss.SSS') +
            frame.timestamp.nsec + ts.format('Z')

          let length = `${frame.rootLayer.payload.length}`
          if (frame.length > frame.rootLayer.payload.length) {
            length += ` (actual: ${frame.length})`
          }
          return <div class="layer-list-view">
            <table>
              <tr><td>Timestamp </td><td>{ tsString }</td></tr>
              <tr><td>Length </td><td>{ length }</td></tr>
            </table>
            { m(LayerItem, {layer: frame.rootLayer}) }
          </div>
        })
      }
    </div>
  }
}
