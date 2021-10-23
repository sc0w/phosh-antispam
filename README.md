# Phosh Calls Anti-Spam

Copyright (C) 2021, Chris Talbot

## About
Phosh Antispam is a program that monitors Gnome calls and automatically hangs up
depending on the user's preferences.

<div align="center">
  <img src="https://gitlab.com/kop316/phosh-antispam/-/raw/master/data/metainfo/screenshot.png?inline=false" width="300" >
</div>

By default, if the number calling you is not in your contact list, or you have the contact listed as "Spam", Phosh AntiSpam will automatically tell GNOME Calls to hang up on the call. This will send it to voicemail. There are additionally these user configuration options:

- Whether to block or allow blocked numbers (also called "Anonymous Number" in GNOME Calls)
- If the caller calls back within 1 minute of the first (blocked) call, whether to allow it through (in case it is a human trying to call back). Note this does not apply to contacts named "Spam", they will always be hung up on.
- If you would like to match a certain type (or types) of number (for example, an area code or a number prefix) to let them through. For example, if you want to allow the area code `201` and the number prefix `312-555-*`, you could allow both combinations (and add others as well).

## License
Unless otherwise specified, all code is licensed under the GPL Version 3 or later
 https://www.gnu.org/licenses/gpl-3.0.en.html

All other images/data is licensed under  Creative Commons
Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)
https://creativecommons.org/licenses/by-sa/4.0/

## Compiling Phosh Antispam
In order to compile Phosh Antispam you need following software packages:

- GCC compiler
- GIO library
- Libhandy Library

Phosh Antispam requires, at a minimum, GNOME Calls version `41.alpha` to work.
If there is a version less than `41.alpha`, Phosh Antispam will not do anything.
To use the `Silence` feature, `41.1` is needed.

## Installing Phosh Antispam
Build tools requirements

When building and testing directly from the repository it is important to
have at least meson version 0.56 or later installed. In addition, some
supported Meson backend is required (e.g. ninja).

To prepare/configure the project for compilation:

    # meson _build

However, if you want to install it, run

    # meson _build -Dprefix=/usr --sysconfdir=/etc

`--sysconfdir=/etc` will install the autostart file in the correct place, and
`-Dprefix=/usr` installs `phosh-antispam` in `/usr/bin/` rather than `/usr/local/bin`

To build the project:

    # meson compile -C _build

Run tests:

    # meson test -C _build

Install:

    # meson install -C _build

    (Install may require sudo or root privileges).

Uninstall:

    # ninja uninstall -C _build/

    (Uninstall may require sudo or root privileges).

## Running from within the source code repository
Meson will build the binary in builddir (_build, using the examples above), and
can be invoked directly.

  Run with debugging

    # G_MESSAGES_DEBUG=all ./_build/src/phosh-antispam

