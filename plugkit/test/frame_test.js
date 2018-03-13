const assert = require('assert')
const { Testing } = require('..')
describe('Frame', () => {
  describe('#leafLayers', () => {
    it('should return frame leafLayers', () => {
      const frame = Testing.createFrameInstance()
      assert.deepEqual([], frame.leafLayers)
    })
  })
})
