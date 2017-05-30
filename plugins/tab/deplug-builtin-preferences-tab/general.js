import { Config, GlobalChannel, Profile, Theme } from 'deplug'
import OptionView from './option-view'
import m from 'mithril'
import objpath from 'object-path'

export default class GeneralView {
  constructor() {
    this.version =
      objpath.get(Config.deplug, 'version', 'n/a')
    this.electronVersion =
      objpath.get(Config.deplug, 'devDependencies.negatron', 'n/a')
    this.themeId = Theme.currentId
  }

  view(vnode) {
    return [
      <h1>General Settings </h1>
      ,
      <table>
        <tr><td>Deplug version</td><td>{ this.version }</td></tr>
        <tr><td>Negatron version</td><td>{ this.electronVersion }</td></tr>
        <tr><td>Theme</td><td>
          <select
          onchange={(event) => {
            const id = event.target.options[event.target.selectedIndex].value
            this.themeId = id
            GlobalChannel.emit('core:theme:set', id)
          }}
          >
          {
            Object.keys(Theme.registry).map((key) => {
              let theme = Theme.registry[key]
              return <option
                selected={this.themeId === theme.id}
                value={ theme.id }
                >{ theme.name }

              </option>
            })
          }
          </select>
        </td></tr>
        {
          Profile.globalOptions.map((opt) => {
            return <tr>
              <td data-tooltip={ `.${opt.id}` }>{ opt.name }</td>
              <td>{ m(OptionView, {option: opt}) }</td>
            </tr>
          })
        }
      </table>
    ]
  }
}
