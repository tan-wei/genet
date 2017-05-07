export default class PcapFile {
  constructor (data) {
    if (data.length < 24) {
      throw new Error('too short global header')
    }
    let littleEndian = true
    let nanosec = false
    const magicNumber = data.readUInt32BE(0, true)
    switch (magicNumber) {
      case 0xd4c3b2a1:
        littleEndian = true
        nanosec = false
        break
      case 0xa1b2c3d4:
        littleEndian = false
        nanosec = false
        break
      case 0x4d3cb2a1:
        littleEndian = true
        nanosec = true
        break
      case 0xa1b23c4d:
        littleEndian = false
        nanosec = true
        break
      default:
        throw new Error('wrong magic_number')
    }

    if (littleEndian) {
      this.versionMajor = data.readUInt16LE(4, true)
      this.versionMinor = data.readUInt16LE(6, true)
      this.thiszone = data.readInt16LE(8, true)
      this.sigfigs = data.readUInt32LE(12, true)
      this.snaplen = data.readUInt32LE(16, true)
      this.network = data.readUInt32LE(20, true)
    } else {
      this.versionMajor = data.readUInt16BE(4, true)
      this.versionMinor = data.readUInt16BE(6, true)
      this.thiszone = data.readInt16BE(8, true)
      this.sigfigs = data.readUInt32BE(12, true)
      this.snaplen = data.readUInt32BE(16, true)
      this.network = data.readUInt32BE(20, true)
    }
    this.versionMinor = data.readUInt16BE(6, true)
    this.versionMinor = data.readUInt16BE(6, true)
    this.versionMinor = data.readUInt16BE(6, true)

    this.packets = []

    let offset = 24
    while (offset < data.length) {
      let tsSec = 0
      let tsUsec = 0
      let inclLen = 0
      let origLen = 0
      if (data.length - offset < 16) {
        throw new Error('too short packet header')
      }
      if (littleEndian) {
        tsSec = data.readUInt32LE(offset, true)
        tsUsec = data.readUInt32LE(offset + 4, true)
        inclLen = data.readUInt32LE(offset + 8, true)
        origLen = data.readUInt32LE(offset + 12, true)
      } else {
        tsSec = data.readUInt32BE(offset, true)
        tsUsec = data.readUInt32BE(offset + 4, true)
        inclLen = data.readUInt32BE(offset + 8, true)
        origLen = data.readUInt32BE(offset + 12, true)
      }

      offset += 16
      if (data.length - offset < inclLen) {
        throw new Error('too short packet body')
      }

      const payload = data.slice(offset, offset + inclLen)
      if (!nanosec) {
        tsUsec *= 1000
      }

      const pakcet = {
        tsSec,
        tsNsec: tsUsec,
        length: origLen,
        payload,
      }
      this.packets.push(pakcet)
      offset += inclLen
    }
  }
}
