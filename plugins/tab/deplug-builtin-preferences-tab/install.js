import { Plugin } from 'deplug'
import denodeify from 'denodeify'
import m from 'mithril'
import npm from 'npm'

export default class InstallView {
  constructor() {
    this.packages = []
  }

  oncreate() {
    this.updatePackages().then((a) => {
      console.log(a)
    })
  }

  async updatePackages() {
    await denodeify(npm.load)({production: true})
    console.log(npm.commands.search)
    npm.config.set('json', true)
    let search = denodeify(npm.commands.search)(['dripcap-'])
    return search
  }

  view(vnode) {
    return [
      <h1>Install Plugins </h1>
      ,
      <table>
        {
          this.packages.map((pkg) => {
            return <tr><td>{ pkg.name }</td><td></td></tr>
          })
        }
      </table>
    ]
  }
}
