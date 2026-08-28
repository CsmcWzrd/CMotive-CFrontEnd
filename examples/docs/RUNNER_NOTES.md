# Runner Notes

`tools/example_build.sh` is the active source of truth for example verification. It reads `manifests/examples.jsonl`, compiles examples with the native C-built CMotive compiler, executes them when requested, checks expected exit codes, rejects stderr, and checks the per-example verification marker when present.

The active Makefile invokes the shell runner only. The former Python runner is retained in `legacy/python-example-tools/` only for source-history/reference.
