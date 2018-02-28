export default {
  config: {
    '_.pcap.snaplen': {
      title: 'Snapshot Length',
      description: 'Snapshot Length',
      type: 'integer',
      minimum: 0,
      default: 2048,
    },
    '_.dissector.confidenceThreshold': {
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
    '_.dissector.concurrency': {
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
    '_.package.registries': {
      title: 'Package Registries',
      type: 'array',
      items: {
        type: 'string',
        format: 'uri',
      },
      uniqueItems: true,
      default: ['https://deplug.net/registry/cache.txt'],
    },
    '_.package.rustpath': {
      title: 'Rust Path',
      type: 'string',
    },
    '_.package.noConfUpdated': {
      title: 'Don\'t Show "Config Updated" Message',
      type: 'boolean',
      default: false,
    },
    '_.debug.enableDebugSession': {
      title: 'Enable Debug Session',
      type: 'boolean',
      default: false,
      description: 'Enabling this option may affect performance',
    },
  },
  workspace: {
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
  },
}
