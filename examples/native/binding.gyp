{
  "targets":[
    {
      "target_name":"plugkit-native-plugin-example",
      "sources":[
        "main.cpp"
      ],
      "include_dirs":[
        "<!(node -e \"require('nan')\")",
        "<!(node -e \"require('../../include')\")"
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
