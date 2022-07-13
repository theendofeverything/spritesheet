# Environment

Windows, install MSYS. Install packages for `make`, `gcc`, and `ctags`.

# Build

```
make q
```

# Run

```
$ ./q.exe
```

# Dependencies

Install MSYS packages for `SDL2`, `SDL2_image`, and `SDL2_ttf`.

# Develop

## Main .c file

The main source is `q.c`. Why `q`? I name code projects
alphabetically and I was up to `q` when I started this project.

## Unity build with header-only libs

This is the only `.c` file. The other C code is in the local `.h`
files included by `q.c`. So the source is a single C file after
the preprocessor step.

Build by running make with the `-B` flag:

```
make -B q
```

`-B` forces `make` to build the recipe, even if `make` sees the
recipe is up to date. This avoids remembering to list the `.h`
local libs as prerequisites in the recipe.

> [!TIP] Build and run from Vim
>
> I build and run from Vim with `;<Space>`. If there are build errors,
> Vim opens the Quickfix List and I press `<Enter>` to jump to the
> first error. Here is the excerpt from my `vimrc`:

```vim
nnoremap <leader><Space> :call BuildAndRunC()<CR><CR>
function BuildAndRunC()
    exec "make -B "..expand("%:t:r")
    let l:build_failed = CheckIfBuildFailed()
    if l:build_failed
        copen
    else
        exec "!./"..expand("%:t:r")..".exe"
    endif
endfunction
```

## Executable has no explicit recipe

In fact, there is no explicit build recipe for `q.exe`. I'm
taking advantage of this nice GNU feature:

```
make blah
(where blah is the stem of a C source file)
```

Does this by default:

```
cc $(CFLAGS) blah.c $(LDLIBS) -o blah
```

So the Makefile is just there to define the `CFLAGS` and
`LDLIBS`.

If there are no compiler and linker flags, then `make` doesn't
even need a Makefile!

## Explicit build recipes for tags

All of the explicit build recipes in the Makefile are related to
generating tags files.

The recipe for `parse-headers.exe` builds a simple string-parsing
utility that converts the preprocessor -M output to the format
expected by ctags. This is how I generate tags for the system
header files. It is also how I split the `show-tags` output into
the two sections: `LIBS` and `q`.

The other recipes run `ctags` in various ways. These are detailed
below.

Tags are really helpful. I use tags in two ways:

- while I code, it is nice to auto-complete and view definitions
- when reading code, it is nice to have a list of all the symbols

## List symbols

Use ctags to print a human-readable list of symbols:

```
make
```

> [!TIP] `make show-tags`
>
> `show-tags` is the first recipe in the Makefile.
> So doing `make` is the same as doing `make show-tags`.

The `show-tags` recipe prints to stdout. It is helpful to open
the `show-tags` output in a Vim window to reference while editing
code. Open the Vim Quickfix List with `:copen`. This displays the
output from the last `make` called from Vim.

```vim
:make
:copen
```

The symbol list is in the order that symbols appear in the code.
The list is split into two sections:

- `LIBS`
    - symbols in the local `.h` includes
- `q`
    - symbols in the main source file

## Tags files

I make two tags files: tags and lib-tags. I put the local code
tags in tags and the system header tags in lib-tags.

### Local tags

Update the tags file for local code with `make tags`.

### System header tags

Update the tags file for system header tags with `make lib-tags`.

Having tags for system header files makes it much easier to
navigate the API for third-party libs. I constantly use the
features described below to look-up function signatures and to
auto-complete symbol names.

I include the library, e.g. `SDL.h`, and run `make lib-tags`. Now
Vim searches the `SDL2` headers when I use:

- `tag` jumping
- `tag-preview`
- `new-omni-completion`

> [!TIP] Vim help
>
> If you are not sure what those Vim features are, look up the
> words in monospace font in the Vim help, e.g.,
>
> `:h new-omni-completion`
