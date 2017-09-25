const assert = require('assert')
const {Testing} = require('../test')

describe('Payload', () => {
  describe('#length', () => {
    it('should return payload length', () => {
      const payload = Testing.createPayloadInstance()
      assert.equal(0, payload.length)
    })
  })
})
