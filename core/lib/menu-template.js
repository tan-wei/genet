import { remote } from 'electron'

const template = [
  {
    label: 'Edit',
    submenu: [
      { role: 'undo' },
      { role: 'redo' },
      { type: 'separator' },
      { role: 'cut' },
      { role: 'copy' },
      { role: 'paste' },
      { role: 'pasteandmatchstyle' },
      { role: 'delete' },
      { role: 'selectall' }
    ],
  },
  {
    label: 'File',
    submenu: [
      {
        label: 'New Window',
        action: 'core:window:new',
        accelerator: 'CommandOrControl+Shift+N',
      },
      {
        label: 'New Live Capture...',
        action: 'core:tab:new-pcap',
        accelerator: 'CommandOrControl+N',
      },
      { type: 'separator' },
      {
        label: 'Display Filter',
        action: 'core:pcap:focus-display-filter',
        accelerator: 'CommandOrControl+F',
      },
      { type: 'separator' },
      {
        label: 'Import File...',
        action: 'core:file:import',
        accelerator: 'CommandOrControl+O',
      },
      {
        label: 'Export File...',
        action: 'core:file:export',
        accelerator: 'CommandOrControl+S',
      },
      { type: 'separator' },
      {
        label: 'Browse User Directory...',
        action: 'core:file:browse-user-dir',
      }
    ],
  },
  {
    label: 'View',
    submenu: [
      { role: 'toggledevtools' },
      {
        label: 'Open Developer Tools for Current Tab',
        action: 'core:tab:open-devtool',
      },
      { type: 'separator' },
      { role: 'resetzoom' },
      { role: 'zoomin' },
      { role: 'zoomout' },
      { type: 'separator' },
      { role: 'togglefullscreen' }
    ],
  },
  {
    role: 'window',
    submenu: [
      { role: 'minimize' },
      { role: 'close' }
    ],
  },
  {
    role: 'help',
    submenu: [
      { label: 'Learn More' }
    ],
  }
]
if (process.platform === 'darwin') {
  template.unshift({
    label: remote.app.getName(),
    submenu: [
      { role: 'about' },
      {
        label: 'Uninstall Helper Tool...',
        action: 'core:pcap:uninstall-helper',
      },
      { type: 'separator' },
      {
        label: 'Preferences...',
        action: 'core:tab:show-preferences',
        accelerator: 'CommandOrControl+,',
      },
      {
        label: 'Packages...',
        action: 'core:tab:show-packages',
        accelerator: 'CommandOrControl+Shift+,',
      },
      { type: 'separator' },
      {
        role: 'services',
        submenu: [],
      },
      { type: 'separator' },
      { role: 'hide' },
      { role: 'hideothers' },
      { role: 'unhide' },
      { type: 'separator' },
      { role: 'quit' }
    ],
  })

  // Edit menu
  template[1].submenu.push(
    { type: 'separator' },
    {
      label: 'Speech',
      submenu: [
        { role: 'startspeaking' },
        { role: 'stopspeaking' }
      ],
    }
  )

  // Window menu
  template[4].submenu = [
    { role: 'close' },
    { role: 'minimize' },
    { role: 'zoom' },
    { type: 'separator' },
    { role: 'front' }
  ]
} else {
  template[0].submenu.unshift(
    {
      label: 'Preferences...',
      action: 'core:tab:show-preferences',
      accelerator: 'CommandOrControl+,',
    },
    {
      label: 'Packages...',
      action: 'core:tab:show-packages',
      accelerator: 'CommandOrControl+Shift+,',
    },
    { type: 'separator' }
  )
}
export default template
