import { remote, shell } from 'electron'
import { Config } from 'deplug'
const contents = remote.getCurrentWebContents()

export default [
  {
    path: ["Edit", "Cut"],
    accelerator: "Cmd+X",
    selector: "cut:",
    click: contents.cut
  },
  {
    path: ["Edit", "Copy"],
    accelerator: "Cmd+C",
    selector: "copy:",
    click: contents.copy
  },
  {
    path: ["Edit", "Paste"],
    accelerator: "Cmd+V",
    selector: "paste:",
    click: contents.paste
  },
  {
    path: ["Edit", "Select All"],
    accelerator: "Cmd+A",
    selector: "selectAll:",
    click: contents.selectAll
  },
  {
    path: ["Developer", "Toggle Developer Tools"],
    accelerator: "Cmd+Shift+I",
    click: () => remote.getCurrentWindow().toggleDevTools()
  },
  {
    path: ["File", "Open Profile Directory..."],
    click: () => shell.showItemInFolder(Config.userProfilePath)
  },
]
