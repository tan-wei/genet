import m from 'mithril'
import moment from 'moment'
import { Channel, Profile } from 'deplug'

export default class StreamView {
  constructor() {
    this.streamId = ''
    this.status = null
    this.chunks = 0
    Channel.on('core:pcap:session-created', (sess) => {
      this.sess = sess
      this.sess.on('listener', (stat) => {
        if (this.chunks < this.status.chunks) {
          for (; this.chunks < this.status.chunks; this.chunks += 1) {

          }
        }
      })
    })
    Channel.on('core:frame:selected', (frames) => {
      this.streamId = ''
      this.namespace = ''
      if (frames.length > 0) {
        const layer = frames[0].primaryLayer
        const streamId = layer.streamId
        if (streamId) {
          this.streamId = streamId.toString('hex')
          this.namespace = layer.namespace
        }
      }
      if (this.streamId) {
        const args = {namespace: this.namespace, streamId: frames[0].primaryLayer.streamId}
        this.sess.setListener('stream', `stream-${this.streamId}`, args)
        this.status = this.sess.getListenerStatus(`stream-${this.streamId}`)
      } else {
        this.sess.setListener('', `stream-${this.streamId}`)
        this.status = null
      }
      m.redraw()
    })
  }

  view(vnode) {
    return <div class="stream-view">
      <ul style={{display: this.streamId ? 'block' : 'none'}}>
        <li>Stream ID: { this.streamId }</li>
        <li>Stream Length: { this.status ? this.status.chunkLength : 0 }</li>
      </ul>
    </div>
  }
}
