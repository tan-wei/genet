const http = require('http')
const express = require('express')
const ws = require('ws')


const data = [
  {
  description: 'Deplug Worker',
  devtoolsFrontendUrl: 'chrome-devtools://devtools/bundled/inspector.html?experiments=true&v8only=true&ws=127.0.0.1:9229/de6d02ae',
  faviconUrl: 'https://github.com/deplug/images/raw/master/deplug.png',
  id: 'de6d02ae',
  title: 'Deplug',
  type: 'node',
  url: 'file://',
  webSocketDebuggerUrl: 'ws://127.0.0.1:9229/de6d02ae',
  }
]

const fields = Symbol('fields')
class Inspector {
  constructor () {
    const app = express()
    app.get('/json', (req, res) => {
      res.send(data)
    })
    const httpServer = http.createServer()
    httpServer.on('request', app)

    const wss = new ws.Server({ server: httpServer })
    wss.on('connection', (ws, req) => {
      console.log(req.url)

        // Connection is up, let's add a simple simple event
        ws.on('message', (message) => {

            // Log the received message and send it back to the client
            console.log('received: %s', message)
            ws.send(`Hello, you sent -> ${message}`)
        })

        // Send immediatly a feedback to the incoming connection
        ws.send('Hi there, I am a WebSocket server')
    })

    httpServer.listen(9229, () => {
    const { host, port } = httpServer.address()
    console.log(`Server is listening on ${host}:${port}`)
  })
 }
}

module.exports = Inspector
new Inspector()
