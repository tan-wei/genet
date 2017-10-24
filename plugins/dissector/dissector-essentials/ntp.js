import { Reader, Layer, Token } from 'plugkit'

const liTable = {
  0: Token`ntp.leapIndicator.noWarning`,
  1: Token`ntp.leapIndicator.sec61`,
  2: Token`ntp.leapIndicator.sec59`,
  3: Token`ntp.leapIndicator.unknown`,
}

const modeTable = {
  0: Token`ntp.mode.reserved`,
  1: Token`ntp.mode.symmetricActive`,
  2: Token`ntp.mode.symmetricPassive`,
  3: Token`ntp.mode.client`,
  4: Token`ntp.mode.server`,
  5: Token`ntp.mode.broadcast`,
  6: Token`ntp.mode.controlMessage`,
  7: Token`ntp.mode.reservedForPrivate`,
}
export default class NTP {
  analyze (ctx, layer) {
    const child = layer.addLayer(ctx, 'ntp')
    child.confidence = Layer.ConfProbable

    if (layer.attr('.src').value !== 123 || layer.attr('.dst').value !== 123) {
      child.confidence = Layer.ConfPossible
    }

    const reader = new Reader(layer.payloads[0].slices[0])

    const head = reader.getUint8()
    const leapIndicator = child.addAttr(ctx, 'ntp.leapIndicator')
    leapIndicator.value = head >> 6
    leapIndicator.type = '@enum'
    leapIndicator.range = reader.lastRange
    leapIndicator.error = reader.lastError

    if (leapIndicator.value in liTable) {
      const attr = child.addAttr(ctx, liTable[leapIndicator.value])
      attr.value = true
      attr.range = reader.lastRange
      attr.error = reader.lastError
    }

    const version = child.addAttr(ctx, 'ntp.version')
    version.value = (head >> 3) & 0b111
    version.type = '@int:dec'
    version.range = reader.lastRange
    version.error = reader.lastError

    const mode = child.addAttr(ctx, 'ntp.mode')
    mode.value = head & 0b111
    mode.type = '@enum'
    mode.range = reader.lastRange
    mode.error = reader.lastError

    if (mode.value in modeTable) {
      const attr = child.addAttr(ctx, modeTable[mode.value])
      attr.value = true
      attr.range = reader.lastRange
      attr.error = reader.lastError
    }

    const stratum = child.addAttr(ctx, 'ntp.stratum')
    stratum.value = reader.getUint8()
    stratum.type = '@int:dec'
    stratum.range = reader.lastRange
    stratum.error = reader.lastError

    const pollInterval = child.addAttr(ctx, 'ntp.pollInterval')
    pollInterval.value = reader.getUint8()
    pollInterval.type = '@int:dec'
    pollInterval.range = reader.lastRange
    pollInterval.error = reader.lastError

    const precision = child.addAttr(ctx, 'ntp.precision')
    precision.value = reader.getUint8()
    precision.type = '@int:dec'
    precision.range = reader.lastRange
    precision.error = reader.lastError

    const rootDelay = child.addAttr(ctx, 'ntp.rootDelay')
    rootDelay.value = reader.getInt16() + (reader.getUint16() / 65536)
    rootDelay.range = reader.lastRange
    rootDelay.error = reader.lastError

    const rootDispersion = child.addAttr(ctx, 'ntp.rootDispersion')
    rootDispersion.value = reader.getInt16() + (reader.getUint16() / 65536)
    rootDispersion.range = reader.lastRange
    rootDispersion.error = reader.lastError

    const ideitifier = child.addAttr(ctx, 'ntp.ideitifier')
    ideitifier.value = reader.slice(0, 4)
    ideitifier.type = stratum.value >= 2
                      ? '@ipv4:addr'
                      : ''
    ideitifier.range = reader.lastRange
    ideitifier.error = reader.lastError

    const fraction = 4294967296

    const referenceTs = child.addAttr(ctx, 'ntp.referenceTs')
    referenceTs.value = reader.getUint32() + (reader.getUint32() / fraction)
    referenceTs.type = '@ntp:time'
    referenceTs.range = reader.lastRange
    referenceTs.error = reader.lastError

    const originateTs = child.addAttr(ctx, 'ntp.originateTs')
    originateTs.value = reader.getUint32() + (reader.getUint32() / fraction)
    originateTs.type = '@ntp:time'
    originateTs.range = reader.lastRange
    originateTs.error = reader.lastError

    const receiveTs = child.addAttr(ctx, 'ntp.receiveTs')
    receiveTs.value = reader.getUint32() + (reader.getUint32() / fraction)
    receiveTs.type = '@ntp:time'
    receiveTs.range = reader.lastRange
    receiveTs.error = reader.lastError

    const transmitTs = child.addAttr(ctx, 'ntp.transmitTs')
    transmitTs.value = reader.getUint32() + (reader.getUint32() / fraction)
    transmitTs.type = '@ntp:time'
    transmitTs.range = reader.lastRange
    transmitTs.error = reader.lastError
  }

  static get layerHints () {
    return ['udp']
  }
}
