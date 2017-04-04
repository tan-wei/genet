import { Channel, Tab } from 'deplug'
Channel.on('core:create-tab', (_, name) => {
  console.log(Tab.get(name))
})
