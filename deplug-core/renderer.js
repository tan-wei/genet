import mithril from 'mithril'

const properties = {}
export default class Renderer {
  static registerProperty (type, renderer) {
    properties[type] = renderer
  }

  static forProperty (type) {
    if (type in properties) {
      return properties[type]
    }
    return properties['']
  }

  static query (object, query) {
    let type = ''
    let value = null
    if (query === '.') {
      value = object
    } else if (query.startsWith('.')) {
      const key = query.substr(1)
      if (key in object) {
        value = object[key]
      }
    }

    if (value === null) {
      if (query === '.id' && typeof object === 'object' &&
        object.constructor.name === 'Frame') {
        value = object.primaryLayer.id
      } else if (typeof object.attr === 'function') {
        const prop = object.attr(query)
        if (prop) {
          ({ type, value } = prop)
        }
      }
    }

    if (query === '.id') {
      type = '@id'
    }

    if (typeof value === 'object' && value.constructor.name === 'Layer') {
      type = value.id
    }

    let renderer = properties['']
    if (type in properties) {
      renderer = properties[type]
    }

    return mithril(renderer, {
      prop: {
        type,
        value,
      },
    })
  }
}
