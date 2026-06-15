# `via`
A minimal vi‑inspired editor written in C11 with ncurses. Fully themable and customizable via `~/.viarc`.

## NOTE: Licensed under GPL-2.0!

## Build

```sh
make
```

Requires `ncurses` and `tinfo` (detected via `pkg-config`, fallback to `-lncurses -ltinfo`).

## Usage

```sh
via [file]
```

### Normal mode

| Key | Action |
|-----|--------|
| `h`/`j`/`k`/`l` / arrows | Move cursor |
| `w`/`b`/`e` | Word motions |
| `W`/`B`/`E` | WORD motions |
| `0`/`$`/`^` | Line start/end/first non‑blank |
| `G`/`gg` | Go to last/first line |
| `{line}G` | Go to line number |
| `%` | Match brackets `()`, `[]`, `{}` |
| `f`/`F`/`t`/`T`{char} | Find/till next/prev char |
| `;`/`,` | Repeat find forward/backward |
| `n`/`N` | Repeat search forward/backward |
| `H`/`M`/`L` | Screen top/middle/bottom |
| `zt`/`zz`/`zb` | Cursor to screen top/mid/bot |
| `Ctrl`+`F`/`B` | Page down/up |
| `Ctrl`+`D`/`U` | Half‑page down/up |
| `Ctrl`+`E`/`Y` | Scroll down/up |

**Insertion**

| Key | Action |
|-----|--------|
| `i` | Insert at cursor |
| `a` | Append after cursor |
| `I` | Insert at first non‑blank |
| `A` | Append at end of line |
| `o`/`O` | Open line below/above |
| `r`{char} | Replace one character |

**Deletion**

| Key | Action |
|-----|--------|
| `x`/`X` | Delete char forward/backward |
| `d`{motion} | Delete motion |
| `dd` | Delete line |
| `D`/`C` | Delete to end of line |
| `s`/`S` | Substitute char/line |

**Operators** (accept motions)

| Key | Action |
|-----|--------|
| `d`{motion} | Delete |
| `c`{motion} | Change (delete + insert) |
| `y`{motion} | Yank (copy) |
| `>`{motion} | Shift right |
| `<`{motion} | Shift left |
| `p`/`P` | Paste after/before |

**Other**

| Key | Action |
|-----|--------|
| `u`/`Ctrl`+`R` | Undo/redo |
| `.` | Repeat last change |
| `~` | Toggle case |
| `J` | Join lines |
| `m`{a-z} | Set mark |
| `'`{a-z} | Jump to mark line |
| `v`/`V` | Visual/line‑wise visual mode |

### Command mode (`:`)

| Command | Action |
|---------|--------|
| `:w` | Write file |
| `:q` | Quit |
| `:q!` | Force quit (discard changes) |
| `:wq`/`:x` | Write and quit |
| `:e <file>` | Edit file |
| `:set <opt>[=<val>]` | Set option |
| `:n` | Repeat last search |
| `:highlight` / `:hi` | Set theme |
| `:s` | Substitute (stub) |

**Search** (`/` and `?`):
- `/pattern` — search forward
- `?pattern` — search backward

### Options (`:set`)

- `tabstop`/`ts` — tab width (default 8)
- `shiftwidth`/`sw` — shift width (default 8)
- `scrolloff`/`so` — scroll offset (default 0)
- `number`/`nu` — toggle line numbers
- `nonumber`/`nonu` — disable line numbers
- `list` — toggle visible whitespace
- `nolist` — disable visible whitespace

### Theme (`~/.viarc`)

Place commands in `~/.viarc`:

```
" comments start with # or "
set number
set ts=4
highlight status fg=black bg=white attr=bold
highlight status-insert fg=black bg=cyan attr=bold
highlight status-visual fg=black bg=yellow attr=bold
highlight cursor-line fg=white bg=blue
highlight selection fg=white bg=blue
highlight linenum fg=yellow bg=black
highlight normal fg=white bg=black
highlight cmdline fg=white bg=black
```

Elements: `normal`, `insert`, `visual`, `status`, `status-insert`, `status-visual`, `cmdline`, `linenum`, `cursor-line`, `selection`, `search`, `eol`

Colors: `black`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`, `default`

Attributes: `bold`, `reverse`, `underline`
