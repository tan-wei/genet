import m from 'mithril'

export default class View {
  view(vnode) {
    return <main>
    <section>
      <h1>A helper tool needs to be installed</h1>
      <p>
        You need the administration password to install it.
      </p>
      <input
        type="button"
        value="Install Helper Tool..."
        onclick={ ()=>{ require('deplug-helper')() } }
      ></input>
    </section>
    <section>
      <h2>How it works</h2>
      <p>
        <ul>
          <li>Creates a new group named ‘access_bpf’</li>
          <li>Puts the current user into the group</li>
        </ul>
        <ul>
          <li>as a previliged user on every startup</li>
          <li>chgrp /dev/bpf*</li>
        </ul>
      </p>
    </section>
    <section>
      <h2>How to uninstall the Helper Tool?</h2>
      <p>
        <ul>
          <li>sudo dscl . -delete /Groups/access_bpf</li>
        </ul>
      </p>
    </section>
    </main>
  }
}
