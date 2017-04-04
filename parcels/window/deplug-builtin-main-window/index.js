import m from 'mithril'

const User = {
  list: [
    {
      firstName: "hello",
      lastName: "world"
    }
  ]
}

export default {
  view: () => {
    return m(".user-list", User.list.map((user) => {
      return m(".user-list-item", user.firstName + " " + user.lastName)
    }))
  }
}
