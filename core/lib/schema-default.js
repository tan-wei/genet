export default {
  config: {
    '_.snaplen': {
      title: 'Snapshot Length',
      description: 'Snapshot Length',
      type: 'integer',
      minimum: 0,
      default: 2048,
    },
    '_.confidenceThreshold': {
      title: 'Confidence Threshold',
      type: 'integer',
      enum: [0, 1, 2, 3],
      enumTitles: [
        '0 - Error',
        '1 - Possible',
        '2 - Probable',
        '3 - Exact'
      ],
      minimum: 0,
      maximum: 3,
      default: 2,
    },
  },
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
