const fs = require('fs-extra')
function getAbiVersion (file) {
  const fd = fs.openSync(file, 'r')
  const marker = Buffer.alloc(1)
  for (let offset = 0; ; offset += 128) {
    if (fs.readSync(fd, marker, 0, 1, offset).bytesRead === 0) {
      break
    }
    if (marker[0] === 0x23) {
      let sub = 1
      for (; ; sub += 1) {
        if (fs.readSync(fd, marker, 0, 1, offset + sub).bytesRead === 0) {
          break
        }
        if (marker[0] !== 0x23) {
          break
        }
      }
      const version = Buffer.alloc(32)
      if (fs.readSync(fd, version, 0, 32, offset + sub).bytesRead === 0) {
        break
      }
      const result = version.toString().match(/:GENET:ABI:(\d+\.\d+):/)
      if (result !== null) {
        return result[1]
      }
    }
  }
  return null
}

module.exports = getAbiVersion
