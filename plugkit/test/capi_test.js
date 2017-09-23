const assert = require('assert')
const result = require('../test').Testing.runCApiTests()

describe('C-API Tests', () => {
  it('should success', () => {
    assert.equal(0, result)
  })
})
