# Visual Studio 2022 package project

Open `vs2022/CMotive.Packages.sln` in Visual Studio 2022 or build it with MSBuild from a VS2022 Developer Command Prompt.

The project is an NMake project. It writes outputs to:

```text
CMotive/build/vs2022/packages/<Configuration>-<Platform>/
```

Expected files include:

```text
bin/cmotive.py
bin/cmotivepp.py
bin/cmotive++.py
obj/package_manager.obj       # when cl.exe/clang-cl.exe is available
cmotive-packages.lib          # when lib.exe is available
BUILD_OUTPUTS.txt
cmotive-packages.stamp
```

Use `BUILD_OUTPUTS.txt` to see exactly what was produced in your environment.


## VS2022 verification update

The Visual Studio 2022 package project files were regenerated as NMake projects.

Verified in this package:

- `.sln` files reference the corrected `.vcxproj` files.
- `.vcxproj` and `.vcxproj.filters` XML parses successfully.
- No corrupted `FULLPath`/helper-script project item remains.
- Output directories are rooted under `build/vs2022/packages/<Configuration>-<Platform>/`.
- Each build writes `BUILD_OUTPUTS.txt` and a `.stamp` file.
