<div align=center>

<img src="https://dwm.suckless.org/dwm.svg"
     data-canonical-src="https://dwm.suckless.org/dwm.svg"
     width="200"
     height="100" />

#### Personal build of Dynamic Window Manager (DWM)
This is suckless, the source code is the documentation! Check out [config.h](config.h).

</div>

## Patches and features

- Layouts:
     - tile
     - bstack
     - spiral
     - dwindle
     - deck
     - monocle
     - centered master
     - centered floating master
     - true fullscreen (prevents focus shifting)
- DWM blocks: Statusbar with my build of [dwmblocks](https://github.com/vladdoster/dwmblocks).
- XRDB: Read colors from xrdb (.Xresources) at run time.
- Scratchpads: Allows you to spawn or restore a floating terminal windows.
- Sticky: A sticky client is visible on all tags.
- Stacker: Move windows up the stack manually (`super-K/J`).
- Shift view: Cycle through tags (`super+g/;`).
- Swap focus: Single shortcut to reach last used window instead of having to think if you should use alt-j or alt-k.
- Vanity gaps: Gaps allowed across all layouts.
- Swallow: If a program run from a terminal would make it inoperable, it temporarily takes its place to save space.
- Color bar: Change the foreground and background color of every statusbar element.
- No border: No border if single window on tag
- Per tag layout: Keeps layout, mwfact, barpos and nmaster per tag.

