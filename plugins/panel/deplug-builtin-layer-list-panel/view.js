import m from 'mithril'
import moment from 'moment'
import { Channel, Profile } from 'deplug'

class PropertyItem {
  view(vnode) {
    const prop = vnode.attrs.property
    const value = (prop.value == null ? '' : prop.value.toString())
    const faClass = `fa ${prop.properties.length ? 'fa-arrow-circle-down' : 'fa-circle-o'}`
    return <li>
      <i class={faClass}></i>
      <label> { prop.name }: </label>
      <span> { prop.summary || value } </span>
      <ul>
        {
          prop.properties.map((prop) => {
            return m(PropertyItem, {property: prop})
          })
        }
      </ul>
    </li>
  }
}

class LayerItem {
  view(vnode) {
    const layer = vnode.attrs.layer
    const faClass = `fa ${layer.children.length ? 'fa-arrow-circle-down' : 'fa-circle-o'}`
    return <ul>
      <li>
        <h4
          data-layer={layer.namespace}
        ><i class={faClass}></i> { layer.name } { layer.summary }</h4>
      </li>
      {
        layer.properties.map((prop) => {
          return m(PropertyItem, {property: prop})
        })
      }
      <li>
        <ul>
          {
            layer.children.map((child) => {
              return m(LayerItem, {layer: child})
            })
          }
        </ul>
      </li>
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
    return this.frames.map((frame) => {
      const profile = Profile.current['$deplug-builtin-layer-list-panel']
      const ts = moment(frame.timestamp)
      const tsString = ts.format(profile.tsformat)
      let length = `${frame.rootLayer.payload.length}`
      if (frame.length > frame.rootLayer.payload.length) {
        length += ` (actual: ${frame.length})`
      }
      let layers = [ frame.rootLayer ]
      if (frame.rootLayer.id === '') {
        layers = frame.rootLayer.children
      }
      return <div class="layer-list-view">
        <ul>
          <li>
            <i class="fa fa-circle-o"></i>
            <label> Timestamp: </label>
            <span> { tsString } </span>
          </li>
          <li>
            <i class="fa fa-circle-o"></i>
            <label> Length: </label>
            <span> { length } </span>
          </li>
        </ul>
        {
          layers.map((layer) => {
            return m(LayerItem, {layer})
          })
        }
      </div>
    })
  }
}
