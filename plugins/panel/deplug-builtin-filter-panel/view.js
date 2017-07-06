import { Channel, Profile, Session } from 'deplug'
import m from 'mithril'

export default class FilterView {
  constructor() {
    this.layerCandidates = {}
    this.candidates = []
    this.activeCandidates = []
    this.showCandidates = false
    this.currentCandidate = -1
  }

  oncreate(vnode) {
    Channel.on('core:display-filter:set', (filter) => {
      vnode.dom.value = filter
    })
    const descriptors = Session.descriptors
    for (const key in descriptors) {
      this.candidates.push({
        propPaths: key,
        name: descriptors[key].name
      })
    }
    this.input = vnode.dom.parentNode.querySelector('input')
    this.list = vnode.dom.parentNode.querySelector('.candidates')
  }

  candidatesFromLayer(layer) {
    if (layer.id.length > 0) {
      this.layerCandidates[layer.id] = layer
    }
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
    case 'Tab':
      if (this.currentCandidate >= 0) {
        this.selectCandidate(this.activeCandidates[this.currentCandidate].propPaths)
        event.preventDefault()
      }
      break
    case 'Enter':
      if (this.currentCandidate >= 0) {
        this.selectCandidate(this.activeCandidates[this.currentCandidate].propPaths)
      } else {
        const filter = event.target.value
        Channel.emit('core:display-filter:set', filter)
      }
      break
    case 'ArrowDown':
      if (this.activeCandidates.length > 0) {
        if (this.currentCandidate < 0) {
          this.currentCandidate = 0
        } else {
          this.currentCandidate = (this.currentCandidate + 1) % this.activeCandidates.length
        }
        this.list.querySelector(`li:nth-child(${this.currentCandidate+1})`)
          .scrollIntoView(false)
      }
      break
    case 'ArrowUp':
      if (this.activeCandidates.length > 0) {
        if (this.currentCandidate < 0) {
          this.currentCandidate = this.activeCandidates.length - 1
        } else {
          this.currentCandidate = (this.currentCandidate +
            this.activeCandidates.length - 1) % this.activeCandidates.length
        }
        this.list.querySelector(`li:nth-child(${this.currentCandidate+1})`)
          .scrollIntoView(false)
      }
      break
    }
    return true
  }

  up(event) {
    switch (event.code) {
    case 'Escape':
    case 'Tab':
    case 'Enter':
      this.currentCandidate = -1
      this.updateCandidates(false)
      break
    default:
    if (this.input.value.length > 0 &&
        this.activeCandidates.length > 0 &&
        this.currentCandidate < 0) {
      this.currentCandidate = 0
      this.list.querySelector(`li:nth-child(${this.currentCandidate+1})`)
        .scrollIntoView(false)
    }
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
    if (!this.showCandidates) {
      this.currentCandidate = -1
    }
  }

  selectCandidate(propPaths) {
    const tokens = this.input.value.split(' ')
    if (tokens.length === 0) {
      tokens.push('')
    }
    tokens[tokens.length - 1] = propPaths
    this.input.value = tokens.join(' ')
    this.updateCandidates(false)
  }

  view(vnode) {
    return [
      <input
        type="text"
        placeholder="Display Filter ..."
        onfocus={ () => this.updateCandidates(true) }
        onblur={ () => this.updateCandidates(false) }
        onkeydown={ (event) => {this.press(event)} }
        onkeyup={ (event) => {this.up(event)} }
      ></input>,
      <div
      class="candidates"
      style={{visibility: this.showCandidates ? 'visible' : 'hidden'}}
      >
        <ul>
          {
            this.activeCandidates.map((cand, index) => {
              return <li
                selected={ this.currentCandidate === index }
              >{ cand.propPaths }
              <small>{ cand.name }</small></li>
            })
          }
        </ul>
      </div>
    ]
  }
}
