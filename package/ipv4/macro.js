module.exports = function ipv4 (str) {
  const sections = str.split('.')
  if (sections.length !== 4) {
    return null
  }
  if (!sections.every((sec) => `${Number.parseInt(sec, 10)}` === sec)) {
    return null
  }
  const addr = sections.map((sec) => Number.parseInt(sec, 10))
  if (!addr.every((sec) => sec >= 0 && sec <= 0xff)) {
    return null
  }
  return JSON.stringify(addr)
}
