{
  "variables":{
    "genet_node_sources":[
      "src/main.cpp"
    ],
    "genet_target": "<!(node -p \"(process.env.NODE_ENV === 'production') ? 'release' : 'debug'\")"
  },
  "target_defaults":{
    "include_dirs":[
      "src"
    ],
    "conditions":[
      [
        "OS=='linux'",
        {
          "libraries":[
            "../../target/<(genet_target)/libgenet_kernel.a",
            "-Wl,-dn,-lpcap,-lcap,-lrt,-dy,-lpthread,-ldl",
          ],
          "cflags_cc":[
            "-fno-strict-aliasing"
          ],
          "defines":[
            "GENET_NODE_OS_LINUX"
          ]
        }
      ],
      [
        "OS=='mac'",
        {
          "libraries":[
            "-L/usr/local/lib",
            "-lpcap",
            "../../target/<(genet_target)/libgenet_kernel.a"
          ],
          "link_settings":{
            "libraries":[
              "$(SDKROOT)/System/Library/Frameworks/Foundation.framework",
              "$(SDKROOT)/System/Library/Frameworks/SystemConfiguration.framework"
            ]
          },
          "xcode_settings":{
            "MACOSX_DEPLOYMENT_TARGET":"10.9",
            "OTHER_CPLUSPLUSFLAGS": [
                "-std=c++11",
                "-stdlib=libc++"
            ],
            "OTHER_LDFLAGS": [
                "-stdlib=libc++"
            ],
          },
          "defines":[
            "GENET_NODE_OS_MAC"
          ]
        }
      ],
      [
        "OS=='win'",
        {
          "libraries": [
            '../../target/<(genet_target)/genet_kernel.lib',
            'Ws2_32.lib',
            'Userenv.lib',
            'Iphlpapi.lib'
          ],
          "defines":[
            "GENET_NODE_OS_WIN",
            "GENET_NODE_DLL_EXPORT",
            "NOMINMAX"
          ],
          "VCCLCompilerTool": {
            'DisableSpecificWarnings': ['4577'],
            'AdditionalOptions': ['/MP', '/d2guard4']
          },
          "VCLinkerTool": {
            'AdditionalOptions': ['/guard:cf']
          }
        }
      ]
    ]
  },
  "targets":[
    {
      "target_name":"genet-node",
      "sources":[
        "<@(genet_node_sources)"
      ],
      "xcode_settings":{
        "GCC_ENABLE_CPP_EXCEPTIONS":"YES"
      },
      "cflags_cc!":[
        "-fno-exceptions"
      ]
    }
  ]
}
