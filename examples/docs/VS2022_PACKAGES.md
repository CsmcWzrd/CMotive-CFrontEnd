# VS2022 Package/Examples Project

Open `vs2022/CMotive.LangExamples.Packages.sln` from Visual Studio 2022 to browse the example package inputs and project metadata.

The active compiler/preprocessor are expected to come from the native C frontend under `../cmotive-source/build/bin/`. Build the frontend first from the source archive, then run the examples from a POSIX/MinGW/clang shell with:

```sh
make check CMOTIVE_ROOT=../cmotive-source
```

The VS project no longer invokes Python helper scripts.
