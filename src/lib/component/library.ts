import BaseLoader from './base'
import { Disposable } from '../disposable'
import genet from '@genet/api'
import glob from 'glob'
import path from 'path'

export namespace LibraryComponent {
  export interface Config {
    main: string
  }

  export class Loader implements BaseLoader {
    private mainFile: string
    private disposable: Disposable

    constructor(comp: Config, dir: string) {
      if (!comp.main) {
        throw new Error('main field required')
      }

      const libs = glob.sync(
        `target/${process.env.GENET_TARGET}/?(lib)${comp.main}.{dll,so,dylib}`,
        { cwd: dir })
      if (libs.length > 0) {
        this.mainFile = path.join(dir, libs[0])
      }
      if (!this.mainFile) {
        throw new Error(`could not resolve ${comp.main} in ${dir}`)
      }
    }
    async load() {
      this.disposable = genet.session.registerLibrary(
        this.mainFile.replace(/\bapp\.asar\b/, 'app.asar.unpacked'))
      return false
    }
    async unload() {
      if (this.disposable) {
        this.disposable.dispose()
      }
      return true
    }
  }
}
