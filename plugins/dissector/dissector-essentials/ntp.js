import { Reader } from 'plugkit';

export default class NTP {
  analyze(ctx, layer) {

  }

  static get layerHints() {
    new Reader();
    return ['udp']
  }
}
