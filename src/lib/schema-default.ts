export default {
  config: {
    '_.pcap.snapshotLength': {
      type: 'integer',
      minimum: 0,
      default: 2048,
    },
    '_.framelist.columns': {
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
    '_.package.rustPath': {
      type: 'string',
    },
    '_.dev.tabReloading': {
      description: 'Touch $HOME/.genet/.reload to reload current tab',
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
