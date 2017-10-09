import { Reader, Layer, Token } from 'plugkit'

const ntpToken = Token.get('ntp')
const srcToken = Token.get('.src')
const dstToken = Token.get('.dst')

const liTable = {
  0: Token.get('noWarning'),
  1: Token.get('sec61'),
  2: Token.get('sec59'),
  3: Token.get('unknown')
}

export default class NTP {
  analyze(ctx, layer) {
    const child = layer.addLayer(ntpToken)
    child.confidence = Layer.ConfProbable

    if (layer.attr(srcToken).value !== 123 || layer.attr(dstToken).value !== 123) {
      child.confidence = Layer.ConfPossible
    }

    const reader = new Reader(layer.payloads[0].slices[0])
  }

  static get layerHints() {
    return ['udp']
  }
}
