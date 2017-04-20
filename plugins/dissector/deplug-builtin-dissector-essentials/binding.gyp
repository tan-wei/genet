{
  "targets":[
    {
      "target_name":"ethernet",
      "sources":[
        "eth.cpp"
      ],
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
    {
      "target_name":"ipv4",
      "sources":[
        "ipv4.cpp"
      ],
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
    }
  ]
}
