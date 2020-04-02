# Luke's build of dwm

Here's my build of dwm.

## FAQ

> What are the bindings?

This is suckless, mmmbud, the source code is the documentation! Check out `config.h`.

## Patches and features

- reads xresources colors
- scratchpad accessible with mod+shift+enter
- fibbonacci/centered master/etc. other layouts
- true fullscreen and prevents focus shifting
- windows can be made sticky
- stacker patch
- shiftview to cycle tags
- gaps around windows and bar
- dwmc

## Please install `libxft-bgra`!

This build of dwm does not block color emoji in the status/info bar, so you must install [libxft-bgra](https://aur.archlinux.org/packages/libxft-bgra/) from the AUR, which fixes a libxft color emoji rendering problem, otherwise dwm will crash upon trying to render one. Hopefully this fix will be in all libxft soon enough.
