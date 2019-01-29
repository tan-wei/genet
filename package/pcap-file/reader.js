module.exports = (sess, url) => {
  if (url.toString().endsWith('.pcap')) {
    sess.createReader('app.genet.reader.pcap-file', url)
    return true
  }
}
