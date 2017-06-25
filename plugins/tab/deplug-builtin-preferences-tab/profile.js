import { Profile } from 'deplug'
import m from 'mithril'

export default class ProfileView {
  view(vnode) {
    return [
      <h1>Profile</h1>
      ,
      <table>
        <tr>
          <td>Current Profile</td>
          <td>{ Profile.currentId }</td>
        </tr>
      </table>
    ]
  }
}
