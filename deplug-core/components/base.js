import objpath from 'object-path'

export default class Component {
  constructor (rootDir, parc, comp) {
    this.type = comp.type
    this.rootDir = rootDir
    this.parc = parc
    this.comp = comp
    this.localExtern = Object.keys(objpath.get(this.parc, 'dependencies', {}))
  }

  async load () {
    // Loading
  }

  async unload () {
    // Unloading
  }
}
