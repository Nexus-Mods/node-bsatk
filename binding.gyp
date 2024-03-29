{
    "targets": [
        {
            "target_name": "bsatk",
            "sources": [
                "bsatk/src/bsaarchive.cpp",
                "bsatk/src/bsaexception.cpp",
                "bsatk/src/bsafile.cpp",
                "bsatk/src/bsafolder.cpp",
                "bsatk/src/bsatypes.cpp",
                "bsatk/src/filehash.cpp",
                "index.cpp"
            ],
            "include_dirs": [
                "<!(node -p \"require('node-addon-api').include_dir\")",
                "./bsatk/src"
            ],
            "dependencies": [
              "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "cflags_cc": ["-std=c++17"],
            "conditions": [
                [
                    'OS=="win"',
                    {
                        "defines": [
                          "UNICODE",
                          "_UNICODE",
                          "NAPI_EXPERIMENTAL",
                          "NAPI_VERSION=<(napi_build_version)"
                        ],
                        "libraries": [
                            "-l../zlib/win32/zlibstatic.lib",
                            "-DelayLoad:node.exe"
                        ],
                        "include_dirs": [
                            "./zlib/include"
                        ],
                        "msvs_settings": {
                            "VCCLCompilerTool": {
                                "ExceptionHandling": 1
                            }
                        },
                        "msbuild_settings": {
                          "ClCompile": {
                            "AdditionalOptions": ["-std:c++17", "/MT"]
                          }
                        }
                    }
                ],
                [
                    'OS=="mac"',
                    {
                        "xcode_settings": {
                            "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
                        }
                    }
                ]
            ]
        }
    ]
}
