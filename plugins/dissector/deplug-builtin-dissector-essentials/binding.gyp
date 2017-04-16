{
  "targets":[
    {
      "target_name":"ethernet",
      "sources":[
        "main.cpp"
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
