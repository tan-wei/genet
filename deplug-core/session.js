import PcapFile from './pcap-file'
import { SessionFactory } from 'plugkit'
import denodeify from 'denodeify'
import fs from 'fs'

const dissectors = []
const linkLayers = []
const samples = []
const descriptors = {}
export default class Session {
  static registerNativeDissector (file) {
    dissectors.push(require(file).dissector)
  }

  static registerDissector (file) {
    dissectors.push(file)
  }

  static registerLinkLayer (layer) {
    linkLayers.push(layer)
  }

  static addDescriptors (desc) {
    Object.assign(descriptors, desc)
  }

  static addSample (samp) {
    samples.push(samp)
  }

  static get dissectors () {
    return dissectors
  }

  static get linkLayers () {
    return linkLayers
  }

  static get samples () {
    return samples
  }

  static get descriptors () {
    return descriptors
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
        if (stat.frames === frames.length) {
          res(sess.getFrames(0, stat.frames))
        }
      })
    })
  }

  static async runSampleBenchmark (options = {}, size = 10000) {
    const pcapData = []
    for (const samp of samples) {
      pcapData.push(denodeify(fs.readFile)(samp.pcap))
    }

    const files = (await Promise.all(pcapData))
    .map((data) => new PcapFile(data))

    const results = []
    for (let sourceId = 0; sourceId < files.length; sourceId += 1) {
      const pcap = files[sourceId]
      const frames = []
      for (const frame of pcap.frames) {
        frames.push({
          link: pcap.network,
          payload: frame.payload,
          length: frame.length,
          timestamp: frame.timestamp,
          sourceId,
        })
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

      // eslint-disable-next-line no-await-in-loop
      const sess = await factory.create()
      sess.analyze(frames)
      const start = process.hrtime()

      // eslint-disable-next-line no-await-in-loop
      const result = await new Promise((res) => {
        sess.on('frame', (stat) => {
          if (stat.frames === frames.length) {
            const diff = process.hrtime(start)
            res({
              frames: stat.frames,
              duration: (diff[0] * 1e9) + diff[1],
              pcap: samples[sourceId].pcap,
            })
          }
        })
      })
      results.push(result)

    }

    return results
  }
}
