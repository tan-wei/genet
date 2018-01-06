const http = require('http')
const express = require('express')
const ws = require('ws')
function endpoints (list, port) {
  return list.map((id) => ({
      description: 'Deplug Worker',
      devtoolsFrontendUrl: 'chrome-devtools://devtools/bundled/inspector.html' +
      `?experiments=true&v8only=true&ws=127.0.0.1:${port}/${id}`,
      faviconUrl: 'https://github.com/deplug/images/raw/master/deplug.png',
      id,
      title: 'Deplug',
      type: 'node',
      url: `deplug://${id}`,
      webSocketDebuggerUrl: `ws://127.0.0.1:${port}/${id}`,
    }))
}

const fields = Symbol('fields')
class Inspector {
  constructor (sess) {
    this[fields] = {
      port: 0,
      connections: new Map(),
    }
    const app = express()
    app.get('/json', (req, res) => {
      res.send(endpoints(sess.inspectors, this[fields].port))
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

      // Send immediatly a feedback to the incoming connection
      //  Ws.send('Hi there, I am a WebSocket server')
    })

  httpServer.listen(9229, '127.0.0.1', () => {
    const { host, port } = httpServer.address()
    this[fields].port = port
    console.log(`Server is listening on ${host}:${port}`)
  })
 }
}

module.exports = Inspector
