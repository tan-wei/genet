const assert = require('assert')
const { Pcap } = require('..')
describe('Pcap', () => {
  describe('.devices', () => {
    it('should return available network devices', () => {
      assert.strictEqual(1, Pcap.devices.length)
    })
  })
  describe('.permission', () => {
    it('should return true if the process has enough permission', () => {
      assert.strictEqual(true, Pcap.permission)
    })
  })
})
