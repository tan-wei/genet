const assert = require('assert')
const {Testing, Token} = require('../test')

describe('Layer', () => {
  describe('#id', () => {
    it('should return layer id', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal('eth', layer.id)
    })
  })
})
