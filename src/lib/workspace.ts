import Config from './config'
import { Disposable } from './disposable'
import objpath from 'object-path'

interface Panel {
  slot: string
}

function flatten(object: object): any[] {
  if (typeof object !== 'object') {
    return object
  }
  let values: any[] = []
  for (const value of Object.values(object)) {
    values = values.concat(flatten(value))
  }
  return values
}

export default class Workspace extends Config {
  private _panels: Map<string, Panel>
  constructor(profile: string) {
    super(profile, 'workspace')
    this._panels = new Map()
  }

  registerPanel(id: string, panel) {
    this._panels.set(id, panel)
    this.update()
    return new Disposable(() => {
      this._panels.delete(id)
    })
  }

  panel(id: string) {
    return this._panels.get(id)
  }

  get panelLayout() {
    return this.get('_.panelLayout', {})
  }

  update() {
    const layout = JSON.parse(JSON.stringify(this.get('_.panelLayout', {})))
    const activePanels = new Set(flatten(layout))
    for (const [id, panel] of this._panels) {
      if (!activePanels.has(id)) {
        objpath.insert(layout, `${panel.slot}.0`, id)
      }
    }
    this.set('_.panelLayout', layout)
  }
}
