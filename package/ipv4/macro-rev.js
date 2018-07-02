module.exports = function ipv4 (attr) {
  return `${Array.from(attr.getValue()).join('.')}`
}
