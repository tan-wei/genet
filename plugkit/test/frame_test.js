const assert = require('assert')
const {Testing, Token} = require('../test')

describe('Frame', () => {
  describe('#timestamp', () => {
    it('should return frame timestamp', () => {
      const frame = Testing.createFrameInstance()
      assert.strictEqual('Date', frame.timestamp.constructor.name)
    })
  })
  describe('#length', () => {
    it('should return frame length', () => {
      const frame = Testing.createFrameInstance()
      assert.strictEqual(0, frame.length)
    })
  })
  describe('#index', () => {
    it('should return frame index', () => {
      const frame = Testing.createFrameInstance()
      assert.strictEqual(0, frame.index)
    })
  })
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
