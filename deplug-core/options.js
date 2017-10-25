export default {
  snaplen: {
    title: 'Snapshot Length (Bytes)',
    type: 'integer',
    minimum: 0,
    default: 2048,
  },
  confidenceThreshold: {
    title: 'Confidence Threshold (%)',
    type: 'integer',
    minimum: 0,
    maximum: 100,
    default: 100,
  },
  noPrebuilt: {
    title: 'Don\'t Use Pre-Built Binaries',
    type: 'boolean',
    default: false,
  },
  concurrency: {
    title: 'Concurrency',
    type: 'integer',
    maximum: 0,
    default: 0,
  },
}
