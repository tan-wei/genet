export default interface BaseLoader {
  load(): Promise<boolean>
  unload(): Promise<boolean>
}
