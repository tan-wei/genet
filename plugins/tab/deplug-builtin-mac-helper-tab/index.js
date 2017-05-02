import { GlobalChannel } from 'deplug'
import { Pcap } from 'plugkit'
import m from 'mithril'

export default class View {
  view(vnode) {
    return <main>
    <section>
      <h1>A helper tool needs to be installed</h1>
      <p>
        <b>TL;DR:</b> macOS does not allow normal users to access pcap devices by default.
        Deplug provides a tiny helper tool to handle this problem.
        Once it has been installed, we can start a live capture without running Deplug as root.
      </p>
      <p>
        Press the follwing button to install the helper tool.
      </p>
      <input
        type="button"
        value={ Pcap.permission ? 'Successfully Installed' : "Install Helper Tool..." }
        disabled={ Pcap.permission }
        onclick={ (event)=>{
          require('deplug-helper')()
          if (Pcap.permission) {
            GlobalChannel.emit('core:tab:open', 'Pcap')
          }
        } }
      ></input>
    </section>
    <section>
      <h2>How it works</h2>
      <p>
        The helper tool installer will do:
        <ol>
          <li>Create a new group ‘access_bpf’</li>
          <li>Put the current user into the group</li>
          <li>Register a startup script</li>
        </ol>
        The startup script runs the following commands as a previliged user every time.
        <ol>
          <li>chgrp /dev/bpf*</li>
          <li>chmod g+rw /dev/bpf*</li>
        </ol>
      </p>
    </section>
    <section>
      <h2>Uninstall the helper tool</h2>
      <ol>
        <li>sudo launchctl unload /Library/LaunchDaemons/net.deplug.DeplugHelper.plist</li>
        <li>sudo rm /Library/LaunchDaemons/net.deplug.DeplugHelper.plist</li>
        <li>sudo rm /Library/PrivilegedHelperTools/net.deplug.DeplugHelper</li>
        <li>sudo dscl . -delete /Groups/access_bpf</li>
      </ol>
    </section>
    </main>
  }
}
