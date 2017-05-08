import { Channel, Session } from 'deplug'
import m from 'mithril'

export default class FilterView {
  constructor() {
    this.layerCandidates = {}
    this.candidates = []
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

  view(vnode) {
    return [
      <input
        type="text"
        placeholder="Display Filter ..."
        onkeypress={ (event) => {this.press(event)} }
      ></input>,
      <div class="candidates">
        <ul>
          {
            this.candidates.map((cand) => {
              return <li>{ cand.propPaths }</li>
            })
          }
        </ul>
      </div>
    ]
  }
}
