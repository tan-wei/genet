module.exports = (sess, arg) => {
    if (arg.file.endsWith('.genet')) {
        sess.createReader('genet-file', arg)
        return true
    }
}
