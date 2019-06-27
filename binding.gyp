{
    "targets": [
        {
            "target_name": "bsatk",
            "includes": [
                "auto.gypi"
            ],
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
                "./zlib/include"
            ],
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            'msbuild_settings': {
              'ClCompile': {
                'AdditionalOptions': ['-std:c++17', '/Ob2', '/Oi', '/Ot', '/Oy', '/GL', '/GF', '/Gy', '/MT']
              }
            },
            "conditions": [
                [
                    'OS=="win"',
                    {
                        "defines!": [
                            "_HAS_EXCEPTIONS=0"
                        ],
                        "libraries": [
                            "-l../zlib/win32/zlibstatic.lib",
                            "-DelayLoad:node.exe",
                        ],
                        "msvs_settings": {
                            "VCCLCompilerTool": {
                                "ExceptionHandling": 1
                            },
                            "VCLibrarianTool": {
                              'AdditionalOptions': [ '/LTCG' ]
                            },
                            'VCLinkerTool': {
                              'LinkTimeCodeGeneration': 1
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
    ],
    "includes": [
        "auto-top.gypi"
    ]
}
