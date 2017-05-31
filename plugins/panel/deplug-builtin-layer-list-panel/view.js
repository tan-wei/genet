import m from 'mithril'
import moment from 'moment'
import { Channel, Profile } from 'deplug'

class BooleanValueItem {
  view(vnode) {
    const faClass = vnode.attrs.value ? 'fa-check-square-o' : 'fa-square-o'
    return <span><i class={`fa ${faClass}`}></i></span>
  }
}

class PropertyValueItem {
  view(vnode) {
    const prop = vnode.attrs.prop
    if (typeof prop.value === 'boolean') {
      return m(BooleanValueItem, {value: prop.value})
    }
    const value = (prop.value == null ? '' : prop.value.toString())
    return <span> { prop.summary || value } </span>
  }
}

class PropertyItem {
  constructor() {
    this.expanded = false
  }

  view(vnode) {
    const prop = vnode.attrs.property
    const value = (prop.value == null ? '' : prop.value.toString())
    let faClass = 'fa fa-circle-o'
    if (prop.properties.length) {
      faClass = this.expanded ? 'fa fa-arrow-circle-down' : 'fa fa-arrow-circle-right'
    }
    return <li>
      <label
        onclick={ () => this.expanded = !this.expanded }
      ><i class={faClass}></i> { prop.name }: </label>
      { m(PropertyValueItem, {prop}) }
      <ul style={{ display: this.expanded ? 'block' : 'none' }}>
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
  constructor() {
    this.expanded = false
  }

  view(vnode) {
    const layer = vnode.attrs.layer
    let faClass = 'fa fa-circle-o'
    if (layer.children.length + layer.properties.length) {
      faClass = this.expanded ? 'fa fa-arrow-circle-down' : 'fa fa-arrow-circle-right'
    }
    return <ul>
      <li>
        <h4
          data-layer={layer.namespace}
          data-layer-error={layer.hasError}
          onclick={ () => this.expanded = !this.expanded }
        ><i class={faClass}></i> { layer.name } { layer.summary } [confidence: { layer.confidence * 100 }%]</h4>
      </li>
      <div style={{ display: this.expanded ? 'block' : 'none' }}>
      {
        layer.properties.map((prop) => {
          return m(PropertyItem, {property: prop})
        })
      }
      </div>
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
