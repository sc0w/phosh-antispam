# Phosh Calls Anti-Spam

Copyright (C) 2021, Chris Talbot

## About
Phosh Antispam is a program that monitors Gnome calls and automatically hangs up
depending on the user's preferences.

## Compiling Phosh Antispam
In order to compile Phosh Antispam you need following software packages:
	- GCC compiler
	- Gio library

Phosh Antispam requires, at a minimum, GNOME Calls version `41.alpha` to work.
If there is a version less than `41.alpha`, Phosh Antispam will not do anything.

## Installing Phosh Antispam
Build tools requirements

When building and testing directly from the repository it is important to
have at least meson version 0.56 or later installed. In addition, some
supported Meson backend is required (e.g. ninja).

To prepare/configure the project for compilation:

    # meson _build --sysconfdir=/etc

(Note: --sysconfdir=/etc is not strictly necessary, but useful if you want
to install Phosh Antispam so the xdg autostart desktop file will install in
the correct place).

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

    # G_MESSAGES_DEBUG=all ./_build/src/phoshspamblock

## General Configuration
You can either run Phosh Antispam and it will create a default template, or you
can copy the following contents to `~/.config/gnome_calls_spam_options`

```
[Spam Block]
AllowBlockedNumbers=true
AllowCallback=true
Matchlist=NULL
```

IMPORTANT NOTE: If you change settings in this file, You must restart Phosh
                anti-spam in order for your settings to take effect!

AllowBlockedNumbers: Whether you want to allow blocked numbers through.

AllowCallback: Allows a ten second timeout for a number to call again and be let through. This can be useful if it is a human calling you.

Matchlist: A comma seperated list of numbers (or partial numbers) you want to allow through. For example, if you want to allow the area code `201` and the number prefix `312-555-*` through, you would make this line:
```
Matchlist=201,312555
```

