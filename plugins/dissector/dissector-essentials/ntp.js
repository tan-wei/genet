import { Reader, Layer, Token } from 'plugkit'

const ntpToken = Token.get('ntp')

export default class NTP {
  analyze(ctx, layer) {
    const child = layer.addLayer(ntpToken)
    child.confidence = Layer.ConfProbable

    const reader = new Reader()
  }

  static get layerHints() {
    return ['udp']
  }
}
