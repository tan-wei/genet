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
    '_.concurrency': {
      title: 'Concurrency',
      type: 'integer',
      enum: [0, 1, 2, 3, 4, 5, 6, 7, 8],
      enumTitles: [
        'Automatic',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8'
      ],
      minimum: 0,
      maximum: 8,
      default: 0,
    },
    '_.packageRegistries': {
      title: 'Package Registries',
      type: 'array',
      items: {
        type: 'string',
        format: 'uri',
      },
      uniqueItems: true,
      default: ['https://deplug.net/registry/list.yml'],
    },
    '_.noPrebuilt': {
      title: 'Don\'t Use Pre-Built Binaries',
      description: 'Force to build native add-ons from source',
      type: 'boolean',
      default: false,
    },
    '_.logLevel': {
      title: 'Log Level',
      type: 'string',
      enum: ['debug', 'info', 'warn', 'error'],
      enumTitles: [
        'Debug',
        'Info',
        'Warning',
        'Error'
      ],
      default: 'debug',
    },
    '_.maxLogLength': {
      title: 'Maximum Log Length',
      type: 'integer',
      minimum: 0,
      default: 1000,
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
