const { Reader, Layer, Token } = require('plugkit')

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
class NTP {
  examine (ctx, layer) {
    if (layer.attr(`${layer.id}.src`).value !== 123 ||
      layer.attr(`${layer.id}.dst`).value !== 123) {
      return Layer.ConfPossible
    }
    return Layer.ConfProbable
  }

  analyze (ctx, layer) {
    const child = layer.addLayer(ctx, 'ntp')
    child.addTag(ctx, 'ntp')
    child.confidence = Layer.ConfProbable

    if (layer.attr(`${layer.id}.src`).value !== 123 ||
        layer.attr(`${layer.id}.dst`).value !== 123) {
      child.confidence = Layer.ConfPossible
    }

    const [parentPayload] = layer.payloads
    const reader = new Reader(parentPayload.slices[0])
    child.range = parentPayload.range

    try {
      const head = reader.getUint8()
      const leapIndicator = child.addAttr(ctx, 'ntp.leapIndicator')
      leapIndicator.value = head >> 6
      leapIndicator.type = '@enum'
      leapIndicator.range = reader.lastRange

      if (leapIndicator.value in liTable) {
        const attr = child.addAttr(ctx, liTable[leapIndicator.value])
        attr.type = '@novalue'
        attr.value = true
        attr.range = reader.lastRange
      }

      const version = child.addAttr(ctx, 'ntp.version')
      version.value = (head >> 3) & 0b111
      version.type = '@int:dec'
      version.range = reader.lastRange

      const mode = child.addAttr(ctx, 'ntp.mode')
      mode.value = head & 0b111
      mode.type = '@enum'
      mode.range = reader.lastRange

      if (mode.value in modeTable) {
        const attr = child.addAttr(ctx, modeTable[mode.value])
        attr.value = true
        attr.type = '@novalue'
        attr.range = reader.lastRange
      }

      const stratum = child.addAttr(ctx, 'ntp.stratum')
      stratum.value = reader.getUint8()
      stratum.type = '@int:dec'
      stratum.range = reader.lastRange

      const pollInterval = child.addAttr(ctx, 'ntp.pollInterval')
      pollInterval.value = reader.getUint8()
      pollInterval.type = '@int:dec'
      pollInterval.range = reader.lastRange

      const precision = child.addAttr(ctx, 'ntp.precision')
      precision.value = reader.getUint8()
      precision.type = '@int:dec'
      precision.range = reader.lastRange

      const rootDelay = child.addAttr(ctx, 'ntp.rootDelay')
      rootDelay.range = [
        reader.lastRange[1],
        reader.lastRange[1] + 4
      ]
      rootDelay.value = reader.getInt16() + (reader.getUint16() / 65536)

      const rootDispersion = child.addAttr(ctx, 'ntp.rootDispersion')
      rootDispersion.range = [
        reader.lastRange[1],
        reader.lastRange[1] + 4
      ]
      rootDispersion.value = reader.getInt16() + (reader.getUint16() / 65536)

      const ideitifier = child.addAttr(ctx, 'ntp.ideitifier')
      ideitifier.value = reader.slice(0, 4)
      ideitifier.type = stratum.value >= 2
        ? '@ipv4:addr'
        : ''
      ideitifier.range = reader.lastRange

      const fraction = 4294967296

      const referenceTs = child.addAttr(ctx, 'ntp.referenceTs')
      referenceTs.range = [
        reader.lastRange[1],
        reader.lastRange[1] + 8
      ]
      referenceTs.value = reader.getUint32() + (reader.getUint32() / fraction)
      referenceTs.type = '@ntp:time'

      const originateTs = child.addAttr(ctx, 'ntp.originateTs')
      originateTs.range = [
        reader.lastRange[1],
        reader.lastRange[1] + 8
      ]
      originateTs.value = reader.getUint32() + (reader.getUint32() / fraction)
      originateTs.type = '@ntp:time'

      const receiveTs = child.addAttr(ctx, 'ntp.receiveTs')
      receiveTs.range = [
        reader.lastRange[1],
        reader.lastRange[1] + 8
      ]
      receiveTs.value = reader.getUint32() + (reader.getUint32() / fraction)
      receiveTs.type = '@ntp:time'

      const transmitTs = child.addAttr(ctx, 'ntp.transmitTs')
      transmitTs.range = [
        reader.lastRange[1],
        reader.lastRange[1] + 8
      ]
      transmitTs.value = reader.getUint32() + (reader.getUint32() / fraction)
      transmitTs.type = '@ntp:time'
    } catch (err) {
      child.addError(ctx, '!out-of-bounds')
      child.confidence = Layer.ConfError
    }
  }

  static get layerHints () {
    return ['udp']
  }
}
module.exports = NTP
