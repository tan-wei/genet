import { Reader, Layer, Token } from 'plugkit'

const liTable = {
  0: Token.get('ntp.leapIndicator.noWarning'),
  1: Token.get('ntp.leapIndicator.sec61'),
  2: Token.get('ntp.leapIndicator.sec59'),
  3: Token.get('ntp.leapIndicator.unknown')
}

export default class NTP {
  analyze(ctx, layer) {
    const child = layer.addLayer('ntp')
    child.confidence = Layer.ConfProbable

    if (layer.attr('.src').value !== 123 || layer.attr('.dst').value !== 123) {
      child.confidence = Layer.ConfPossible
    }

    const reader = new Reader(layer.payloads[0].slices[0])

    const first = reader.getUint8()
    const leapIndicator = child.addAttr('ntp.leapIndicator')
    leapIndicator.value = first >> 6
    leapIndicator.type = '@enum'

    if (leapIndicator.value in liTable) {
      const attr = child.addAttr(liTable[leapIndicator.value])
      attr.value = true
    }
  }

  static get layerHints() {
    return ['udp']
  }
}
