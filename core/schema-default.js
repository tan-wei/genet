export default {
  layout: {
    '_.window.size': {
      type: 'array',
      items: {
        type: 'integer',
        minimum: 100,
      },
      default: [1200, 600],
    },
    '_.window.titleBarStyle': {
      type: 'string',
      default: 'hidden-inset',
    },
    '_.window.vibrancy': {
      type: 'string',
      default: 'ultra-dark',
    },
  },
}
