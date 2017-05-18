import PcapFile from './pcap-file'
import { SessionFactory } from 'plugkit'
import denodeify from 'denodeify'
import fs from 'fs'

const dissectors = []
const streamDissectors = []
const linkLayers = []
const samples = []
export default class Session {
  static registerDissector (file) {
    dissectors.push(require(file).factory)
  }

  static registerStreamDissector (file) {
    streamDissectors.push(require(file).factory)
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

  static async runSampleAnalysis () {
    const pcapData = []
    for (const samp of samples) {
      pcapData.push(denodeify(fs.readFile)(samp.pcap))
    }

    const files = (await Promise.all(pcapData))
      .map((data) => new PcapFile(data))

    const factory = new SessionFactory()
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
    for (const pcap of files) {
      for (const frame of pcap.frames) {
          frames.push({
            link: pcap.network,
            payload: frame.payload,
            length: frame.length,
            timestamp: frame.timestamp,
            sourceId: frames.length,
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
}
