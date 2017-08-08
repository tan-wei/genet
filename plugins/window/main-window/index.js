export default {
  components: [
    {
      type: 'core:window',
      window: {
        less: 'theme.less',
        root: 'view.js'
      }
    },
    {
      type: 'core:menu',
      menu: {
        root: 'menu.js'
      }
    }
  ]
}
