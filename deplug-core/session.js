import PcapFile from './pcap-file'
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
    console.log(files)
  }
}
