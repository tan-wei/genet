{
  "target_defaults": {
      "include_dirs":[
        "<!(node -e \"require('nan')\")",
        "<!(node -e \"require('../../../node_modules/plugkit/include')\")"
      ],
      "conditions":[
        [
          "OS=='mac'",
          {
            "xcode_settings":{
              "MACOSX_DEPLOYMENT_TARGET":"10.9"
            }
          }
        ]
      ]
  },
  "targets":[
    {
      "target_name":"ethernet",
      "sources":[
        "eth.cpp"
      ]
    },
    {
      "target_name":"ipv4",
      "sources":[
        "ipv4.cpp"
      ]
    },
    {
      "target_name":"tcp",
      "sources":[
        "tcp.cpp"
      ]
    }
  ]
}
