export default class PcapFileImportHandler {
  async loadFrames(stream) {
    const data = await new Promise((res, rej) => {
      let buf = Buffer.alloc(0)
      stream.on('data', (chunk) => {
        buf = Buffer.concat([buf, chunk])
      })
      stream.on('end', (chunk) => {
        res(buf)
      })
      stream.on('error', (error) => {
        rej(error)
      })
    })

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

    let versionMajor = 0
    let versionMinor = 0
    let thiszone = 0
    let sigfigs = 0
    let snaplen = 0
    let network = 0
    if (littleEndian) {
      versionMajor = data.readUInt16LE(4, true)
      versionMinor = data.readUInt16LE(6, true)
      thiszone = data.readInt16LE(8, true)
      sigfigs = data.readUInt32LE(12, true)
      snaplen = data.readUInt32LE(16, true)
      network = data.readUInt32LE(20, true)
    } else {
      versionMajor = data.readUInt16BE(4, true)
      versionMinor = data.readUInt16BE(6, true)
      thiszone = data.readInt16BE(8, true)
      sigfigs = data.readUInt32BE(12, true)
      snaplen = data.readUInt32BE(16, true)
      network = data.readUInt32BE(20, true)
    }

    const frames = []

    let offset = 24
    while (offset < data.length) {
      let tsSec = 0
      let tsUsec = 0
      let inclLen = 0
      let origLen = 0
      if (data.length - offset < 16) {
        throw new Error('too short frame header')
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
        throw new Error('too short frame body')
      }

      const payload = data.slice(offset, offset + inclLen)
      if (!nanosec) {
        tsUsec *= 1000
      }

      const date = new Date((tsSec * 1000) + Math.floor(tsUsec / 1000000))
      date.nsec = tsUsec % 1000000

      frames.push({
        length: origLen,
        timestamp: date,
        payload
      })

      offset += inclLen
    }

    return {
      pcap: {
        snaplen,
        link: network
      },
      frames
    }
  }
}
