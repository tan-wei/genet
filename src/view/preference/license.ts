import fs from 'fs'
import m from 'mithril'
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

export default class License {
  view(vnode) {
    const nodes = [
      m('h4', ['genet']),
      m('pre', [license])
    ]
    const { active } = vnode.attrs
    if (active) {
      nodes.push(m(LicenseInfo, {}))
    }
    return nodes
  }
}

const license = `MIT License

Copyright (c) 2018

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.`