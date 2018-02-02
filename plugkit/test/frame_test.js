const assert = require('assert')
const { Testing } = require('..')
describe('Frame', () => {
  describe('#sourceId', () => {
    it('should return frame sourceId', () => {
      const frame = Testing.createFrameInstance()
      assert.strictEqual(0, frame.sourceId)
    })
  })
  describe('#leafLayers', () => {
    it('should return frame leafLayers', () => {
      const frame = Testing.createFrameInstance()
      assert.deepEqual([], frame.leafLayers)
    })
  })
})
