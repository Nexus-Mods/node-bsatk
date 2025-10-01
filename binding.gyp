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
                "index.cpp",
                "string_cast.h",
                "bsatk/src/bsaarchive.h",
                "bsatk/src/bsaexception.h",
                "bsatk/src/bsafile.h",
                "bsatk/src/bsafolder.h",
                "bsatk/src/bsatk.h",
                "bsatk/src/bsatype.h",
                "bsatk/src/bsatypes.h",
                "bsatk/src/errorcodes.h",
                "bsatk/src/filehash.h",
                "bsatk/src/semaphore.h"
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
            "cflags_cc": ["-std=c++20"],
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
                            "AdditionalOptions": ['/std:c++20', '/Zc:__cplusplus', "/MT"]
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
