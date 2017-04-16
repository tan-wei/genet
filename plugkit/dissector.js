import { Layer, Chunk, Property } from 'plugkit';

export default class Dissector {
  constructor(v) {

  }

  analyze(layer) {
    let child = new Layer();
    child.confidence = 5;
    let c = new Chunk();
    c.payload = Buffer.from('aaaaa', 'utf8')
    c.streamId = "aaaa/aaaa" + Math.random()
    c.streamNamespace = "eth ipv4 tcp"
    child.addChunk(c);
    let p = new Property()
    p.value = ["Buffer.from('trtrt', 'utf8')"]
    child.addProperty(p)
    return child;
  }

  static get namespaces() {
    return [/<.+>/]
  }
}
