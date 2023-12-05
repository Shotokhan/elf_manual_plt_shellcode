# elf_manual_plt_shellcode
Example of run-time manual PLT in C.

TODO: ... description of what it does and usage ...

## Version
Tested with the following version of `gcc`:

```
gcc (GCC) 12.3.1 20230508 (Red Hat 12.3.1-1)
```

If you encounter issues with other versions and want to make them work, open an *Issue* for the specific version: public discussion can help everyone. You can also just open an *Issue* for a different version that works well, putting in the `.text` area as output of `objdump -D ./bin/main`. The same for other compilers and/or architectures.

> After doing any changes, make sure that `_start` is the first function in the `.text` area of the compiled binary, that the code does not make use of global area and that it's statically linked.

## Techniques
TODO: ... description of code techniques and compilation options ...

TODO: ... open a *Pull Request* if you are able to make the `FNV1A_32_HASH` directive work at compile-time.

## Related
TODO: argument the related projects
Another approach for libc: `dlopen`, `dlsym`.

For kernel: `kallsyms_lookup_name`
