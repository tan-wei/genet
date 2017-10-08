import { Reader, Layer, Token } from 'plugkit'

const ntpToken = Token.get('ntp')
const srcToken = Token.get('.src')
const dstToken = Token.get('.dst')

export default class NTP {
  analyze(ctx, layer) {
    const child = layer.addLayer(ntpToken)
    child.confidence = Layer.ConfProbable

    if (layer.attr(srcToken).value !== 123 || layer.attr(dstToken).value !== 123) {
      child.confidence = Layer.ConfPossible
    }

    const reader = new Reader()
  }

  static get layerHints() {
    return ['udp']
  }
}
