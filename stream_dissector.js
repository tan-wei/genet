import { Layer, Chunk, Property } from 'plugkit';

export default class Dissector {
  constructor() {
  }

  analyze(chunk) {
    let child = new Layer();
    child.confidence = 5;
    let c = new Chunk();
    c.payload = Buffer.from('aaaaa', 'utf8')
    c.streamId = "aaaa/aaaa" + Math.random()
    c.streamNamespace = "eth ipv4 tcp@@@"
    child.addChunk(c);
    return child;
  }

  static get namespaces() {
    return ["eth ipv4 tcp"]
  }
}
