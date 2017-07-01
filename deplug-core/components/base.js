import objpath from 'object-path'

export default class Component {
  constructor (rootDir, pkg, comp) {
    this.type = comp.type
    this.rootDir = rootDir
    this.pkg = pkg
    this.comp = comp
    this.localExtern = Object.keys(objpath.get(this.pkg, 'dependencies', {}))
  }

  async load () {
    // Loading
  }

  async unload () {
    // Unloading
  }
}
