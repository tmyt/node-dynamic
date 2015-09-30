{
  "target_defaults":
    {
      "include_dirs" : [ "<!(node -e \"require('nan')\")" ]
    },
  "targets": [
    {
      "target_name": "dynamic",
      "sources": [ "src/dynamic.cc", "src/module.cc" ]
    }
  ]
}
