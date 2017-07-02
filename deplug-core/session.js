import PcapFile from './pcap-file'
import { SessionFactory } from 'plugkit'
import denodeify from 'denodeify'
import fs from 'fs'

const dissectors = []
const streamDissectors = []
const linkLayers = []
const samples = []
export default class Session {
  static registerNativeDissector (file) {
    dissectors.push(require(file).factory)
  }

  static registerNativeStreamDissector (file) {
    streamDissectors.push(require(file).factory)
  }

  static registerDissector (file) {
    dissectors.push(file)
  }

  static registerStreamDissector (file) {
    streamDissectors.push(file)
  }

  static registerLinkLayer (layer) {
    linkLayers.push(layer)
  }

  static addSample (samp) {
    samples.push(samp)
  }

  static get dissectors () {
    return dissectors
  }

  static get streamDissectors () {
    return streamDissectors
  }

  static get linkLayers () {
    return linkLayers
  }

  static get samples () {
    return samples
  }

  static async runSampleAnalysis (options = {}) {
    const pcapData = []
    for (const samp of samples) {
      pcapData.push(denodeify(fs.readFile)(samp.pcap))
    }

    const files = (await Promise.all(pcapData))
      .map((data) => new PcapFile(data))

    const factory = new SessionFactory()
    factory.options = options
    for (const layer of Session.linkLayers) {
      factory.registerLinkLayer(layer)
    }
    for (const diss of Session.dissectors) {
      factory.registerDissector(diss)
    }
    for (const diss of Session.streamDissectors) {
      factory.registerStreamDissector(diss)
    }

    const sess = await factory.create()

    const frames = []
    for (let sourceId = 0; sourceId < files.length; sourceId += 1) {
      const pcap = files[sourceId]
      for (const frame of pcap.frames) {
          frames.push({
            link: pcap.network,
            payload: frame.payload,
            length: frame.length,
            timestamp: frame.timestamp,
            sourceId,
          })
      }
    }
    sess.analyze(frames)

    return new Promise((res) => {
      sess.on('frame', (stat) => {
        if (stat.frames >= frames.length && stat.queue === 0) {
          res(sess.getFrames(0, stat.frames))
        }
      })
    })
  }

  static async runSampleBenchmark (options = {}, size = 100000) {
    const pcapData = []
    for (const samp of samples) {
      pcapData.push(denodeify(fs.readFile)(samp.pcap))
    }

    const files = (await Promise.all(pcapData))
      .map((data) => new PcapFile(data))

    const frames = []
    for (let sourceId = 0; sourceId < files.length; sourceId += 1) {
      const pcap = files[sourceId]
      for (const frame of pcap.frames) {
          frames.push({
            link: pcap.network,
            payload: frame.payload,
            length: frame.length,
            timestamp: frame.timestamp,
            sourceId,
          })
      }
    }

    const source = [].concat(frames)
    for (let conut = 0; frames.length < size; conut += 1) {
      frames.push(source[conut % source.length])
    }

    const factory = new SessionFactory()
    factory.options = options
    for (const layer of Session.linkLayers) {
      factory.registerLinkLayer(layer)
    }
    for (const diss of Session.dissectors) {
      factory.registerDissector(diss)
    }
    for (const diss of Session.streamDissectors) {
      factory.registerStreamDissector(diss)
    }

    const sess = await factory.create()
    sess.analyze(frames)
    const start = process.hrtime()
    return new Promise((res) => {
      sess.on('frame', (stat) => {
        if (stat.frames >= frames.length && stat.queue === 0) {
          const diff = process.hrtime(start)
          res([
            {
              frames: stat.frames,
              duration: (diff[0] * 1e9) + diff[1],
            }
          ])
        }
      })
    })
  }
}
