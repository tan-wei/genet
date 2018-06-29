module.exports = function mac (str) {
  if (!(/^(?:[0-9a-f]{2}:){5}[0-9a-f]{2}$/i).test(str)) {
    return null
  }
  return JSON.stringify(str.split(':').map((sec) => Number.parseInt(sec, 16)))
}
