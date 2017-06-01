import m from 'mithril'
import moment from 'moment'
import { Channel, Profile } from 'deplug'
import Buffer from 'buffer'

class BooleanValueItem {
  view(vnode) {
    const faClass = vnode.attrs.value ? 'fa-check-square-o' : 'fa-square-o'
    return <span><i class={`fa ${faClass}`}></i></span>
  }
}

class DateValueItem {
  view(vnode) {
    const profile = Profile.current['$deplug-builtin-layer-list-panel']
    const ts = moment(vnode.attrs.value)
    const tsString = ts.format(profile.tsformat)
    return <span>{ tsString }</span>
  }
}

class BufferValueItem {
  view(vnode) {
    const buffer = vnode.attrs.value
    const hex = buffer.slice(0, 16).toString('hex') +
      (buffer.length > 16 ? '...' : '')
    return <span>0x{ hex }</span>
  }
}

class PropertyValueItem {
  view(vnode) {
    const prop = vnode.attrs.prop
    if (typeof prop.value === 'boolean') {
      return m(BooleanValueItem, {value: prop.value})
    } else if (prop.value instanceof Date) {
      return m(DateValueItem, {value: prop.value})
    } else if (prop.value instanceof Uint8Array) {
      return m(BufferValueItem, {value: prop.value})
    }
    const value = (prop.value == null ? '' : prop.value.toString())
    return <span> { value } </span>
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
      <label
      class="summary"
      style={{ display: prop.summary.length && prop.summary != prop.value ? 'inline' : 'none' }}
      >{ prop.summary }</label>
      <label
      class="error"
      style={{ display: prop.error ? 'inline' : 'none' }}
      ><i class="fa fa-exclamation-triangle"></i> { prop.error }</label>
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
        ><i class={faClass}></i> { layer.name } { layer.summary }
        <span
        style={{ display: layer.confidence < 1.0 ? 'inline' : 'none' }}
        ><i class="fa fa-question-circle"></i> { layer.confidence * 100 }%</span>
        </h4>
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
