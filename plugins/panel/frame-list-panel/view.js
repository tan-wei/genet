import throttle from 'lodash.throttle'
import m from 'mithril'
import { Channel, Panel, Profile, Session, Renderer, Tab } from 'deplug'

class FrameItem {
  constructor() {

  }

  oninit(vnode) {
    this.frame = vnode.attrs.session.getFrames(vnode.attrs.seq - 1, 1)[0]
  }

  select() {
    Channel.emit('core:frame:selected', [this.frame])
  }

  view(vnode) {
    const index = vnode.attrs.index
    const itemHeight = vnode.attrs.itemHeight
    return <div
      class="frame-item"
      data-layer={this.frame.primaryLayer.tags.join(' ')}
      data-selected={ vnode.attrs.selected }
      onmousedown={() => {this.select()}}
      style={{
        height: `${itemHeight}px`,
        top: `${index * itemHeight}px`
      }}
    >
      <div class="frame-column">{ Renderer.query(this.frame, '.index') }</div>
      <div class="frame-column">{ Renderer.query(this.frame, '.id') }</div>
      <div class="frame-column">{ Renderer.query(this.frame, '.src') }</div>
      <div class="frame-column">{ Renderer.query(this.frame, '.dst') }</div>
      <div class="frame-column">{ Renderer.query(this.frame, '.length') }</div>
      <div class="frame-column">{ Renderer.query(this.frame, '.primaryLayer') }</div>
    </div>
  }
}

export default class FrameListView {
  constructor() {
    const mapResolution = Profile.current.get('frame-list-panel', 'mapResolution')

    this.frame = {frames: 0}
    this.session = null
    this.viewScrollTop = 0
    this.viewHeight = 0
    this.mapHeight = mapResolution
    this.previousScrollTop = 0
    this.selectedFrames = []

    Channel.on('core:frame:selected', (frames) => {
      this.selectedFrames = frames
      m.redraw()
    })

    this.columns = [
      {name: 'No', value: 'seq'},
      {name: 'Protocol', value: 'primaryLayer.name'},
      {name: 'Length', value: 'length'},
      {name: 'Summary', value: 'primaryLayer.summary'},
    ]
    this.attrs = {
      'layer-confidence': 'primaryLayer.confidence',
      'frame-length': 'length',
      'frame-capture-length': 'rootLayer.payload.length',
    }

    this.updateMapThrottle = throttle((vnode) => {
      this.updateMap(vnode)
    }, 200)

    Channel.on('core:pcap:session-created', (sess) => {
      this.session = sess
      this.filtered = null
      this.session.on('frame', (stat) => {
        this.frame = stat
        m.redraw()
      })
      this.session.on('filter', (stat) => {
        this.filtered = stat.main
        m.redraw()
      })
      m.redraw()
    })

    Channel.on('core:display-filter:set', (filter) => {
      if (this.session) {
        this.session.setDisplayFilter('main', filter)
      }
    })
  }

  oncreate(vnode) {
    this.frameView = vnode.dom.parentNode.querySelector('.frame-list-view')
    this.style = vnode.dom.parentNode.querySelector('.scrollbar-style')
    this.onupdate(vnode)
    this.frameView.addEventListener('scroll', (event) => {
      this.viewHeight = event.target.offsetHeight
      this.viewScrollTop = event.target.scrollTop
      m.redraw()
    })
  }

  onupdate(vnode) {
    this.viewHeight = this.frameView.offsetHeight
    this.viewScrollTop = this.frameView.scrollTop
    this.updateMapThrottle(vnode)

    const maxScrollTop = this.frameView.scrollHeight - this.frameView.clientHeight
    if (this.previousScrollTop <= this.frameView.scrollTop) {
      this.frameView.scrollTop = maxScrollTop
      this.previousScrollTop = maxScrollTop
    } else {
      this.previousScrollTop =
        Math.max(this.previousScrollTop, this.frameView.scrollHeight * 0.8)
    }
  }

  updateMap(vnode) {
    if (this.session && this.frame.frames > 0) {
      const dummy = vnode.dom.parentNode.querySelector('.dummy-item')
      const ctx = dummy.getContext('2d')
      const frames = (this.filtered ? this.filtered.frames : this.frame.frames)
      ctx.clearRect(0, 0, 1, this.mapHeight)

      if (frames > 0) {
        for (let i = 0; i < this.mapHeight; ++i) {
          let index = Math.floor(frames / this.mapHeight * (i + 0.5))
          if (this.filtered) {
            index = this.session.getFilteredFrames('main', index, 1)[0] - 1
          }
          const frame = this.session.getFrames(index, 1)[0]
          dummy.setAttribute('data-layer', frame.primaryLayer.tags.join(' '))
          ctx.fillStyle = getComputedStyle(dummy, null).getPropertyValue("background-color")
          ctx.fillRect(0, i, 1, 1)
        }
      }
      const data = dummy.toDataURL("image/png")

      this.style.textContent = `
      .padding, .frame-list-view::-webkit-scrollbar {
        background-image: url(${data});
      }
      `
    }
  }

  view(vnode) {
    const frames = (this.filtered ? this.filtered.frames : this.frame.frames)
    const itemHeight = 30
    const viewHeight = frames * itemHeight
    const margin = 5
    const begin = Math.max(0,
      Math.floor(Math.min(this.viewScrollTop, viewHeight - this.viewHeight) / itemHeight) - margin)
    const end = Math.min(begin +
      Math.ceil(this.viewHeight / itemHeight) + margin * 2, frames)

    let filterdFrames = null
    if (this.filtered) {
      filterdFrames = this.session.getFilteredFrames('main', begin, (end - begin))
    }

    return [
      <style class="scrollbar-style"></style>
      ,
      <canvas
        style="opacity: 0; position: absolute;"
        class="dummy-item"
        data-layer="eth ipv4 tcp"
        width="1"
        height={this.mapHeight}
      ></canvas>
      ,
      <div class="header">
        <div class="frame-column">No.</div>
        <div class="frame-column">Protocol</div>
        <div class="frame-column">Source</div>
        <div class="frame-column">Destination</div>
        <div class="frame-column">Length</div>
        <div class="frame-column">Summary</div>
      </div>
      ,
      <div class="frame-list-view">
        <div class="padding"
          style={{height: `${viewHeight}px`}}
        >
          {
            (new Array(end - begin)).fill().map((dev, index) => {
              const id = filterdFrames ? filterdFrames[index] : (index + begin + 1)
              const selected = this.selectedFrames.some((frame) => frame.index === id )
              return m(FrameItem, {
                key: id,
                seq: id,
                index: index + begin,
                itemHeight,
                columns: this.columns,
                attrs: this.attrs,
                session: this.session,
                selected
              })
            })
          }
        </div>
      </div>
    ]
  }
}
