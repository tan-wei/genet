import { GlobalChannel } from 'deplug'
import { remote } from 'electron'
const { dialog } = remote

export default [
  /*
  {
    path: ["File", "Open Pcap File..."],
    accelerator: "Cmd+O",
    click: () => {
      let files = dialog.showOpenDialog(remote.getCurrentWindow(), {
        filters: [{name: 'PCAP File', extensions: ['pcap']}]
      })
      if (files) {
        GlobalChannel.emit('core:tab:open', 'Pcap', {files})
      }
    }
  }
  */
]
