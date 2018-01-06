const http = require('http')
const express = require('express')
const ws = require('ws')

const fields = Symbol('fields')
class InspectorServer {
  constructor (sess) {
    this[fields] = {
      port: 0,
      connections: new Map(),
      sess,
    }
    const app = express()
    app.get('/json', (req, res) => {
      res.send(this.sessions)
    })
    const httpServer = http.createServer()
    httpServer.on('request', app)

    sess.setInspectorCallback((msg) => {
      const sock = this[fields].connections.get(msg.id)
      if (sock) {
        sock.send(msg.msg)
      }
    })

    const wss = new ws.Server({ server: httpServer })
    wss.on('connection', (sock, req) => {
      const id = req.url.substr(1)
      this[fields].connections.set(id, sock)
      sock.on('message', (message) => {
        sess.sendInspectorMessage(id, message)
      })
    })

  httpServer.listen(0, '127.0.0.1', () => {
    const { port } = httpServer.address()
    this[fields].port = port
  })
 }

 get sessions () {
   const { port, sess } = this[fields]
   return sess.inspectors.map((id) => ({
      description: 'Deplug Worker',
      devtoolsFrontendUrl:
        'chrome-devtools://devtools/bundled/inspector.html' +
        `?experiments=true&v8only=true&ws=127.0.0.1:${port}/${id}`,
      faviconUrl: 'https://github.com/deplug/images/raw/master/deplug.png',
      id,
      title: 'Deplug',
      type: 'node',
      url: `deplug://${id}`,
      webSocketDebuggerUrl: `ws://127.0.0.1:${port}/${id}`,
    }))
  }
}

module.exports = InspectorServer
