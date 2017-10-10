import { Reader, Layer, Token } from 'plugkit'

const liTable = {
  0: Token`ntp.leapIndicator.noWarning`,
  1: Token`ntp.leapIndicator.sec61`,
  2: Token`ntp.leapIndicator.sec59`,
  3: Token`ntp.leapIndicator.unknown`
}

const modeTable = {
  0: Token`ntp.mode.reserved`,
  1: Token`ntp.mode.symmetricActive`,
  2: Token`ntp.mode.symmetricPassive`,
  3: Token`ntp.mode.client`,
  4: Token`ntp.mode.server`,
  5: Token`ntp.mode.broadcast`,
  6: Token`ntp.mode.controlMessage`,
  7: Token`ntp.mode.reservedForPrivate`
}

export default class NTP {
  analyze(ctx, layer) {
    const child = layer.addLayer('ntp')
    child.confidence = Layer.ConfProbable

    if (layer.attr('.src').value !== 123 || layer.attr('.dst').value !== 123) {
      child.confidence = Layer.ConfPossible
    }

    const reader = new Reader(layer.payloads[0].slices[0])

    const head = reader.getUint8()
    const leapIndicator = child.addAttr('ntp.leapIndicator')
    leapIndicator.value = head >> 6
    leapIndicator.type = '@enum'
    leapIndicator.range = reader.lastRange
    leapIndicator.error = reader.lastError

    if (leapIndicator.value in liTable) {
      const attr = child.addAttr(liTable[leapIndicator.value])
      attr.value = true
      attr.range = reader.lastRange
      attr.error = reader.lastError
    }

    const version = child.addAttr('ntp.version')
    version.value = (head >> 3) & 0b111
    version.range = reader.lastRange
    version.error = reader.lastError
  }

  static get layerHints() {
    return ['udp']
  }
}
