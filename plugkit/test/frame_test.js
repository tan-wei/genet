const assert = require('assert')
const {Testing, Token} = require('../test')

describe('Frame', () => {
  describe('#timestamp', () => {
    it('should return frame timestamp', () => {
      const frame = Testing.createFrameInstance()
      assert.equal('Date', frame.timestamp.constructor.name)
    })
  })
})
