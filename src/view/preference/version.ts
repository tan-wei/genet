import Env from '../../lib/env'
import native from '@genet/load-module'
import fs from 'fs'
import m from 'mithril'
import objpath from 'object-path'
import path from 'path'
import zlib from 'zlib'

class LicenseInfo {
  private license: any[] | null
  constructor() {
    this.license = null
  }

  oncreate() {
    this.loadLicenseFile()
  }

  view() {
    if (this.license === null) {
      return []
    }
    return [
      m('h4', ['Third-Party Software']),
      m('p', { class: 'license' }, this.license.map((item) => m('details', [
        m('pre', [item.body]),
        m('summary', [item.name])
      ])))
    ]
  }

  loadLicenseFile() {
    const licenseFile =
      path.resolve(__dirname, '../../..',
        'genet_modules/src/asset/license.json.gz')
    fs.readFile(licenseFile, (err, data) => {
      if (!err) {
        zlib.gunzip(data, (gerr, json) => {
          if (!gerr) {
            this.license = JSON.parse(json.toString("utf8"))
            m.redraw()
          }
        })
      }
    })
  }
}

export default class Plugin {
  private readonly version: string

  constructor() {
    this.version = objpath.get(Env.genet, 'version', 'n/a')
  }
  view(vnode) {
    const nodes = [
      m('h4', ['genet version']),
      m('span', [this.version]),
      m('h4', ['ABI version']),
      m('span', [native.version.abi])
    ]
    const { active } = vnode.attrs
    if (active) {
      nodes.push(m(LicenseInfo, {}))
    }
    return nodes
  }
}
