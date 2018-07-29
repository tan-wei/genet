export default {
  config: {
    '_.pcap.snaplen': {
      title: 'Snapshot Length',
      description: 'Snapshot Length',
      type: 'integer',
      minimum: 0,
      default: 2048,
    },
    '_.framelist.columns': {
      title: 'FrameList Columns',
      type: 'array',
      default: [
        {
          name: 'Source',
          value: '_.src',
        },
        {
          name: 'Destination',
          value: '_.dst',
        },
        {
          name: 'Length',
          value: 'link.length',
        }
      ],
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
      title: 'Don\'t Show "Reload to apply" Message',
      type: 'boolean',
      default: false,
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
