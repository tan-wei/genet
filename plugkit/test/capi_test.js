const assert = require('assert')
const result = require('..').Testing.runCApiTests()
describe('C-API Tests', () => {
  it('should success', () => {
    assert.strictEqual(0, result)
  })
})
