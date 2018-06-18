import env from '../../lib/env'
import fs from 'fs'
import m from 'mithril'
import objpath from 'object-path'
import path from 'path'
import zlib from 'zlib'

class LicenseInfo {
  constructor () {
    this.license = null
  }

  oncreate () {
    this.loadLicenseFile()
  }

  view () {
    if (this.license === null) {
      return []
    }
    return [
      m('h4', ['License Information']),
      m('p', { class: 'license' }, this.license.map((item) => m('details', [
        m('pre', [item.body]),
        m('summary', [item.name])
      ])))
    ]
  }

  loadLicenseFile () {
    const licenseFile =
      path.resolve(__dirname, '../../..',
        'genet-modules/core/asset/license.json.gz')
    fs.readFile(licenseFile, (err, data) => {
      if (!err) {
        zlib.gunzip(data, (gerr, json) => {
          if (!gerr) {
            this.license = JSON.parse(json)
            m.redraw()
          }
        })
      }
    })
  }
}

export default class Plugin {
  constructor () {
    this.version = objpath.get(env.genet, 'version', 'n/a')
    this.electronVersion =
      objpath.get(env.genet, 'devDependencies.negatron', 'n/a')
  }
  view (vnode) {
    const nodes = [
      m('h4', ['Deplug version']),
      m('span', [this.version]),
      m('h4', ['Negatron version']),
      m('span', [this.electronVersion])
    ]
    const { active } = vnode.attrs
    if (active) {
      nodes.push(m(LicenseInfo, {}))
    }
    return nodes
  }
}
