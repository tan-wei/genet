import { GlobalChannel } from 'deplug'
import { remote } from 'electron'
const { dialog } = remote

async function readPcapFile() {

}

export default class Menu {
  action() {
    let filePath = dialog.showOpenDialog(remote.getCurrentWindow(), {
      filters: [{name: 'PCAP File', extensions: ['pcap']}]
    })
    if (files !== null) {
      GlobalChannel.emit('core:tab:open', 'Pcap', {files})
    }
  }
}
