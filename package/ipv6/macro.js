module.exports = function ipv6(str) {
  const sections = str.split(':')
  if (sections[sections.length - 1] === '') {
    sections.pop()
  }
  if (sections[0] === '') {
    sections.shift()
  }
  const pad = sections.indexOf('')
  if (pad >= 0) {
    sections.splice(pad, 1, ...(new Array(9 - sections.length)).fill('0'))
  }
  if (sections.length !== 8) {
    return null
  }
  const addr = sections.map((sec) => Number.parseInt(sec, 16))
  if (!addr.every((sec) => sec >= 0 && sec <= 0xffff)) {
    return null
  }
  const array = addr.map((sec) => [Math.floor(sec / 256), sec % 256])
    .reduce((lhs, rhs) => lhs.concat(rhs))
  return `0x${array.map((num) => (`0${num.toString(16)}`).slice(-2)).join('')}`
}
