import { Channel, Session } from 'deplug'
import m from 'mithril'

export default class FilterView {
  constructor() {
    this.layerCandidates = {}
    this.candidates = []
    this.activeCandidates = []
    this.showCandidates = false
  }

  oncreate(vnode) {
    Channel.on('core:display-filter:set', (filter) => {
      vnode.dom.value = filter
    })
    Session.runSampleAnalysis().then((frames) => {
      for (const frame of frames) {
        this.candidatesFromLayer(frame.rootLayer)
      }
      for (const propPaths in this.layerCandidates) {
        this.candidates.push({
          propPaths,
          name:  this.layerCandidates[propPaths].name
        })
      }
      m.redraw()
    })
    this.input = vnode.dom.parentNode.querySelector('input')
  }

  candidatesFromLayer(layer) {
    for (const prop of layer.properties) {
      this.candidatesFromProperty([layer.id], prop)
    }
    for (const child of layer.children) {
      this.candidatesFromLayer(child)
    }
  }

  candidatesFromProperty(paths, prop) {
    const propPaths = paths.concat(prop.id)
    this.layerCandidates[propPaths.join('.')] = prop
    for (const child of prop.properties) {
      this.candidatesFromProperty(propPaths, child)
    }
  }

  press(event) {
    switch (event.code) {
    case 'Enter':
      const filter = event.target.value
      Channel.emit('core:display-filter:set', filter)
    }
    return true
  }

  up(event) {
    switch (event.code) {
    case 'Enter':
      this.updateCandidates(false)
      break
    default:
      this.updateCandidates(true)
    }
    return true
  }

  updateCandidates(active) {
    const tokens = this.input.value.split(' ')
    const input = tokens[tokens.length - 1]
    this.activeCandidates = this.candidates.filter((cand) => {
      return cand.propPaths.startsWith(input)
    })
    this.showCandidates = active && !!this.activeCandidates.length
  }

  view(vnode) {
    return [
      <input
        type="text"
        placeholder="Display Filter ..."
        onfocus={ () => this.updateCandidates(true) }
        onblur={ () => this.updateCandidates(false) }
        onkeypress={ (event) => {this.press(event)} }
        onkeyup={ (event) => {this.up(event)} }
      ></input>,
      <div
      class="candidates"
      style={{visibility: this.showCandidates ? 'visible' : 'hidden'}}
      >
        <ul>
          {
            this.activeCandidates.map((cand) => {
              return <li>{ cand.propPaths } <small>{ cand.name }</small></li>
            })
          }
        </ul>
      </div>
    ]
  }
}
