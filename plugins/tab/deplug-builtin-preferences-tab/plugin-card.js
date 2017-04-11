import m from 'mithril'

export default class PluginCard {
  view(vnode) {
    let name = vnode.attrs.pkg.name
      .replace(/^(deplug-builtin-|deplugin-)/, '')
    return <div>
      <a>{ name }</a>
      <p>
        <label><input type="checkbox" name="enabled"></input>Enabled</label>
      </p>
    </div>
  }
}
