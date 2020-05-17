<div align=center>

<img src="https://dwm.suckless.org/dwm.svg" 
     data-canonical-src="https://dwm.suckless.org/dwm.svg"
     width="200"
     height="100" />
  
#### Personal build of Dynamic Window Manager (DWM)
This is suckless, the source code is the documentation! Check out [config.h](config.h).

</div>

## Patches and features

- Clickable statusbar with my build of [dwmblocks](https://github.com/vladdoster/dwmblocks).
- Reads Xresources colors/variables
- Scratchpads: Enables multiple scratchpads, each with one asigned window.
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
- Sticky: A sticky client is visible on all tags.
- Stacker: Move windows up the stack manually (`super-K/J`).
- Shiftview: Cycle through tags (`super+g/;`).
- Vanity gaps: Adds (inner) gaps between client windows and (outer) gaps between windows and the screen edge.
- Swallow: Adds "window swallowing" to dwm as known from Plan 9's windowing system rio.
- Color bar: change the foreground and background color of every statusbar element.
- No Border: Remove the border when there is only one window visible.
- Per tag layout: Keeps layout, mwfact, barpos and nmaster per tag.
