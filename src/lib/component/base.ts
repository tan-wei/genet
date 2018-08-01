export default interface BaseComponent {
  load() : Promise<boolean>
  unload() : Promise<boolean>
}
