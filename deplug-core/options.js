export default [
  {
    id: 'snaplen',
    name: 'Snapshot Length (Bytes)',
    type: 'integer',
    min: 0,
    default: 2048,
  },
  {
    id: 'confidenceThreshold',
    name: 'Confidence Threshold (%)',
    type: 'integer',
    min: 0,
    max: 100,
    default: 100,
  },
  {
    id: 'noPrebuilt',
    name: 'Don\'t Use Pre-Built Binaries',
    type: 'boolean',
    default: false,
  },
  {
    id: 'concurrency',
    name: 'Concurrency',
    type: 'integer',
    min: 0,
    default: 0,
  }
]
