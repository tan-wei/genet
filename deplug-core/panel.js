const panelList = {}
export default class Panel {
  static mount (name, slot, component, less) {
    if (!(slot in panelList)) {
      panelList[slot] = []
    }
    panelList[slot].push({
      name,
      component,
      less,
    })
  }

  static get (slot) {
    return panelList[slot] || []
  }
}
