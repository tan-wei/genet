const map = new WeakMap()
export default function internal (object) {
  if (!map.has(object)) {
    map.set(object, {})
  }
  return map.get(object)
}
