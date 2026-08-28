# CMotive VS2022 Native Frontend Fix Verification

Date: 2026-08-28
Version: 0.2.4-cfrontend

## User-reported VS2022 errors addressed

- `C2632 'char' followed by 'char' is illegal` at `src/native/cmotivetool.c:69`
  - Cause: Windows headers can define `small`, conflicting with the local `char small[4096]` buffer.
  - Fix: renamed the local buffer to `stackbuf`.

- `C2371 'accept': redefinition; different basic types` and follow-on `accept(...)` warnings/errors
  - Cause: parser helper function `accept(Parser*, const char*)` collided with Winsock `accept(SOCKET, sockaddr*, int*)` from Windows headers.
  - Fix: renamed the parser helper to `parser_accept` and updated all parser call sites.

- MSVC pointer/int warnings around `strtok_r`
  - Cause: `strtok_r` is POSIX and not declared by MSVC C mode.
  - Fix: Windows builds alias `strtok_r` to MSVC `strtok_s`.

- Windows runtime temporary file path
  - Cause: generated C temporary file used hard-coded `/tmp`.
  - Fix: Windows builds now use `%TEMP%`, `%TMP%`, or `.`.

## Verification performed in this sandbox

```sh
make -f Makefile.linux clean all test full-test
```

Result:

```text
CMotive compiler 0.2.4-cfrontend
CMotive tests: PASS
CMotive tests: PASS
```

A Windows-header compatibility syntax check was also run with fake Windows headers that define `small` and declare Winsock `accept()`. The patched `src/native/cmotivetool.c` passed syntax checking with those Windows collision conditions enabled.

## Note

A real VS2022 compile could not be executed inside this Linux sandbox, but the specific MSVC compile failures in the supplied log have been directly eliminated in the source.
