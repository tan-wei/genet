const m = require('mithril')
class StreamView {
  constructor () {
    this.sess = null
    this.selectedFrame = null
    this.streamId = null
    deplug.action.on('core:session:created', (sess) => {
      this.sess = sess
      sess.on('filter', () => {
        m.redraw()
      })
    })
    deplug.action.on('core:frame:selected', (frames) => {
      this.selectedFrame = frames[0] || null
      this.streamId = null
      let filter = ''
      if (this.selectedFrame !== null) {
        const streamIdKey = `${this.selectedFrame.primaryLayer.id}.streamId`
        const streamIdAttr = this.selectedFrame.query(streamIdKey)
        if (streamIdAttr !== null) {
          this.streamId = streamIdAttr.value
          filter = `${streamIdKey} === ${this.streamId}`
        }
      }
      this.sess.setDisplayFilter('stream-filter', filter)
      m.redraw()
    })
  }

  view () {
    if (this.sess === null || this.selectedFrame === null) {
      return m('div', { class: 'view' }, [
        'No frame selected'
      ])
    }
    if (this.streamId === null || !('stream-filter' in this.sess.filter)) {
      return m('div', { class: 'view' }, [
        'No streams detected'
      ])
    }

    const count = this.sess.filter['stream-filter'].frames
    const indices = this.sess.getFilteredFrames('stream-filter', 0, count)
    const frames = indices.map((index) => this.sess.getFrames(index - 1, 1)[0])
    return m('div', { class: 'view' }, frames.map((frame) => m('ul', [
        frame.primaryLayer.payloads.map(
          (payload) => payload.slices.map(
            (slice) => m('li', [`${slice.length} bytes`, payload.type])))
      ])))
  }
}

module.exports = StreamView
