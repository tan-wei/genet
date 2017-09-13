import m from 'mithril'
import moment from 'moment'
import objpath from 'object-path'
import { Channel, Profile, Session, Renderer } from 'deplug'
import Buffer from 'buffer'

class BooleanValueItem {
  view(vnode) {
    const faClass = vnode.attrs.value ? 'fa-check-square-o' : 'fa-square-o'
    return <span><i class={`fa ${faClass}`}></i></span>
  }
}

class DateValueItem {
  view(vnode) {
    const tsformat = Profile.current.get('layer-list-panel', 'tsformat')
    const ts = moment(vnode.attrs.value)
    const tsString = ts.format(tsformat)
    return <span>{ tsString }</span>
  }
}

class BufferValueItem {
  view(vnode) {
    const maxLen = 6
    const buffer = vnode.attrs.value
    const hex = buffer.slice(0, maxLen).toString('hex') +
      (buffer.length > maxLen ? '...' : '')
    return <span>[{buffer.length} bytes] 0x{ hex }</span>
  }
}

class ArrayValueItem {
  view(vnode) {
    return <ul>{vnode.attrs.value.map((value) => {
      return <li>{m(PropertyValueItem, {
        prop: {value}
      })}</li>
    })}</ul>
  }
}

class ObjectValueItem {
  view(vnode) {
    const obj = vnode.attrs.value
    return <ul>{Object.keys(obj).map((key) => {
      return <li><label>{ key }</label>{m(PropertyValueItem, {
        prop: {value: obj[key]}
      })}</li>
    })}</ul>
  }
}

class LayerValueItem {
  view (vnode) {
    const layer = vnode.attrs.value
    if (layer.payloads.length) {
      return <span> [{ layer.id }] Payloads </span>
    }
    return <span> [{ layer.id }] </span>
  }
}

class PropertyValueItem {
  view(vnode) {
    const prop = vnode.attrs.prop
    if (prop.value === null) {
      return <span></span>
    } else if (typeof prop.value === 'boolean') {
      return m(BooleanValueItem, {value: prop.value})
    } else if (prop.value instanceof Date) {
      return m(DateValueItem, {value: prop.value})
    } else if (prop.value instanceof Uint8Array) {
      return m(BufferValueItem, {value: prop.value})
    } else if (Array.isArray(prop.value)) {
      return m(ArrayValueItem, {value: prop.value})
    } else if (typeof prop.value === 'object' &&
        prop.value.constructor.name === 'Layer') {
      return m(LayerValueItem, {value: prop.value})
    } else if (typeof prop.value === 'object' &&
        prop.value != null &&
        Reflect.getPrototypeOf(prop.value) === Object.prototype) {
      return m(ObjectValueItem, {value: prop.value})
    }
    const value = (prop.value == null ? '' : prop.value.toString())
    if (value.length > 1024) {
      return <span>
        <details>
          <summary>{value.substr(0, 64)}... ({value.length})</summary>
          { value }
        </details>
      </span>
    }
    return <span> { value } </span>
  }
}

Renderer.registerProperty('', PropertyValueItem)

function selectRange(range = []) {
  Channel.emit('core:frame:range-selected', range)
}

const propSymbol = Symbol('prop')
const orderSymbol = Symbol('order')

function orderedProperties(obj) {
  return Object.values(obj)
    .sort((a, b) => { return a[orderSymbol] - b[orderSymbol] })
    .map((item) => {return item })
}

class PropertyItem {
  view(vnode) {
    const prop = vnode.attrs.property[propSymbol]
    const value = (prop.value == null ? '' : prop.value.toString())
    const children = orderedProperties(vnode.attrs.property)
    let faClass = 'property'
    if (children.length) {
      faClass = 'property children'
    }
    const range = [
      prop.range[0] + vnode.attrs.dataOffset,
      prop.range[1] + vnode.attrs.dataOffset
    ]
    const name = (prop.id in Session.attributes) ?
      Session.attributes[prop.id].name : prop.id
    const propRenderer = Renderer.forProperty(prop.type)

    return <li
        data-range={ `${range[0]}:${range[1]}` }
        onmouseover={ () => selectRange(range) }
        onmouseout= { () => selectRange() }
      >
      <details>
        <summary class={ faClass }>
          <label>
          <i class="fa fa-circle-o"> </i>
          <i class="fa fa-arrow-circle-right"> </i>
          <i class="fa fa-arrow-circle-down"> </i>
           { name }: </label>
          { m(propRenderer, {prop, layer: vnode.attrs.layer}) }
          <label
          class="error"
          style={{ display: prop.error ? 'inline' : 'none' }}
          ><i class="fa fa-exclamation-triangle"></i> { prop.error }</label>
        </summary>
        <ul>
          {
            children.map((prop) => {
              return m(PropertyItem, {
                property: prop,
                layer: vnode.attrs.layer
              })
            })
          }
        </ul>
      </details>
    </li>
  }
}

class LayerItem {
  view(vnode) {
    const layer = vnode.attrs.layer
    let dataOffset = 0
    let dataLength = 0
    if (layer.parent) {
      const parentPayload = layer.parent.payloads[0]
      const parentAddr = parentPayload.slices[0].addr
      let rootPayload = parentPayload
      for (let parent = layer.parent.parent; parent; parent = parent.parent) {
        rootPayload = parent.payloads[0]
      }
      const rootAddr = rootPayload.slices[0].addr
      dataLength = parentPayload.slices[0].length
      dataOffset = parentAddr[1] - rootAddr[1]
    }
    const range = [
      dataOffset,
      dataOffset + dataLength
    ]
    const layerId = layer.id
    const name = (layerId in Session.attributes) ?
      Session.attributes[layerId].name : layerId

    const propObject = {[layerId]: {}}
    const properties = layer.properties
    for (let i = 0; i < properties.length; ++i) {
      const prop = properties[i]
      let id = prop.id
      if (id.startsWith('.')) {
        id = layer.id + id
      }
      objpath.ensureExists(propObject, id, {})
      const item = objpath.get(propObject, id)
      item[propSymbol] = prop
      item[orderSymbol] = i
    }

    return <ul>
      <li
        data-range={ `${range[0]}:${range[1]}` }
        onmouseover={ () => selectRange(range) }
        onmouseout= { () => selectRange() }
      >
      <details>
        <summary class="layer children" data-layer={layer.tags.join(' ')}>
          <i class="fa fa-arrow-circle-right"> </i>
          <i class="fa fa-arrow-circle-down"> </i>
          { name } { Renderer.query(layer, '.') }
          <span
          style={{ display: layer.streamId > 0 ? 'inline' : 'none' }}
          ><i class="fa fa-exchange"></i> Stream #{ layer.streamId }</span>
          <span
          style={{ display: layer.confidence < 1.0 ? 'inline' : 'none' }}
          ><i class="fa fa-question-circle"></i> { layer.confidence * 100 }%</span>
        </summary>
        {
          orderedProperties(propObject[layerId]).map((prop) => {
            return m(PropertyItem, {
              property: prop,
              layer: layer,
              dataOffset
            })
          })
        }
        <a>Payloads</a>
        <ul>
        {
          layer.payloads.map((payload) => {
            return payload.slices.map((slice) => {
              return <li> { m(BufferValueItem, {value: slice}) } : { payload.type } </li>
            })
          })
        }
        </ul>
      </details>
      </li>
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
      const tsformat = Profile.current.get('layer-list-panel', 'tsformat')
      const ts = moment(frame.timestamp)
      const tsString = ts.format(tsformat)
      let length = `${frame.rootLayer.payloads[0].slices[0].length}`
      if (frame.length > frame.rootLayer.payloads[0].slices[0].length) {
        length += ` (actual: ${frame.length})`
      }
      let layers = [ frame.rootLayer ]
      const rootId = frame.rootLayer.id
      if (rootId.startsWith('[')) {
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
