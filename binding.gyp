
{
    "targets": [{
        "target_name": "nodecanal",
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions"],
        "sources": [
            "cppsrc/main.cpp",
            "cppsrc/samples/functionexample.cpp",
            "cppsrc/samples/actualclass.cpp",
            "cppsrc/samples/classexample.cpp",
            "src/node-canal.cpp",
            "src/canalif.cpp"
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")",
            "src/"
        ],
        'libraries': [
            "-lrt",
            "-lm",
            "-lpthread",
            "-ldl",
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
        
    }]
}