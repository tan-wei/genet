import BaseComponent from './base'
import fs from 'fs'
import glob from 'glob'
import objpath from 'object-path'
import path from 'path'

export default class LibraryComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    const file = objpath.get(comp, 'main', '')
    if (!file) {
      throw new Error('main field required')
    }

    const searchPaths = ['.']
    for (const spath of searchPaths) {
      const absolute = path.join(dir, spath, file)
      if (fs.existsSync(absolute)) {
        this.mainFile = absolute
        break
      }
    }
    if (!this.mainFile) {
      const releaseLibs = glob.sync(
        `crates/${file}/target/release/*.{dll,so,dylib}`,
        { cwd: dir })
      const debugLibs = glob.sync(
        `crates/${file}/target/debug/*.{dll,so,dylib}`,
        { cwd: dir })
      if (releaseLibs.length > 0) {
        this.mainFile = path.join(dir, releaseLibs[0])
      } else if (debugLibs.length > 0) {
        this.mainFile = path.join(dir, debugLibs[0])
      }
    }
    if (!this.mainFile) {
      throw new Error(`could not resolve ${file} in ${dir}`)
    }
  }
  async load () {
    this.disposable = genet.session.registerLibrary(
      this.mainFile.replace(/\bapp\.asar\b/, 'app.asar.unpacked'))
    return false
  }
  async unload () {
    if (this.disposable) {
      this.disposable.dispose()
      this.disposable = null
      return false
    }
    return true
  }
}
