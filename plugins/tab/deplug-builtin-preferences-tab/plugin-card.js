import m from 'mithril'

export default class PluginCard {
  view(vnode) {
    const pkg = vnode.attrs.pkg
    let name = pkg.name
      .replace(/^(deplug-builtin-|deplugin-)/, '')
    return <div class="card">
      <a>{ name } <small>({ pkg.version })</small></a>
      <p>
        <label><input type="checkbox" name="enabled" checked></input>Enabled</label>
      </p>
    </div>
  }
}
