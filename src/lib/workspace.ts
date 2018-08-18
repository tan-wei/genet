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

export default class Workspace {
  private _config: Config
  private _panels: Map<string, Panel>
  constructor(profile: string) {
    this._config = new Config(profile, 'workspace')
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
    return this._config.get('_.panelLayout', {})
  }

  get(id: string, defaultValue?: any) {
    return this._config.get(id, defaultValue)
  }

  set(id: string, value: any) {
    this._config.set(id, value)
  }

  del(id: string) {
    this._config.del(id)
  }

  private update() {
    const layout = JSON.parse(JSON.stringify(this._config.get('_.panelLayout', {})))
    const activePanels = new Set(flatten(layout))
    for (const [id, panel] of this._panels) {
      if (!activePanels.has(id)) {
        objpath.insert(layout, `${panel.slot}.0`, id)
      }
    }
    this._config.set('_.panelLayout', layout)
  }
}
