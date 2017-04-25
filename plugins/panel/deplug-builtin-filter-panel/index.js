import { Channel } from 'deplug'
import m from 'mithril'

export default class FilterView {
  constructor() {

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
    return <input
      type="text"
      onkeypress={ (event) => {this.press(event)} }
    ></input>
  }
}
