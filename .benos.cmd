cmd_benos := aarch64-linux-gnu-ld  -p --no-undefined -X -o benos -T arch/arm64/kernel/benos.lds -Map benos.map arch/arm64/kernel/head.o  init/built-in.lib --start-group  kernel/built-in.lib  mm/built-in.lib  arch/arm64/kernel/built-in.lib  arch/arm64/mach-rpi/built-in.lib  lib/built-in.lib  --end-group 