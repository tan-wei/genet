import { remote, shell, ipcRenderer } from 'electron'
import { Config, GlobalChannel, Profile } from 'deplug'

export default ([
  {
    path: ["Edit", "Cut"],
    accelerator: "Cmd+X",
    selector: "cut:"
  },
  {
    path: ["Edit", "Copy"],
    accelerator: "Cmd+C",
    selector: "copy:"
  },
  {
    path: ["Edit", "Paste"],
    accelerator: "Cmd+V",
    selector: "paste:"
  },
  {
    path: ["Edit", "Select All"],
    accelerator: "Cmd+A",
    selector: "selectAll:"
  },
  {
    path: ["Developer", "Toggle Developer Tools"],
    accelerator: "Cmd+Shift+I",
    click: () => remote.getCurrentWindow().toggleDevTools()
  },
  {
    path: ["File", "Import..."],
    click: () => GlobalChannel.emit('core:file:import'),
    accelerator: "CmdOrCtrl+O"
  },
  {
    path: ["File", "Open Profile Directory..."],
    click: () => shell.showItemInFolder(Config.userProfilePath)
  },
  {
    path: [
      (process.platform === 'darwin') ? remote.app.getName() : "Edit",
      "Preferences..."
    ],
    accelerator: "CmdOrCtrl+,",
    click: () => GlobalChannel.emit('core:tab:open', 'Preferences')
  }
]).concat(Profile.list.map((id) => {
  return {
    path: [
      (process.platform === 'darwin') ? remote.app.getName() : "Window",
      "New Window",
      id
    ],
    click: () => ipcRenderer.send('new-window', id)
  }
}))
