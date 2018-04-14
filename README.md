# About

This directory contains the source code for a tiny init devised for
the Pygos system.

The main goal of this project is to create a simple framework for:

- system boot up and initialization
- service supervision

With the additional aims of having something that:

- simply works
- is easy to understand
- is easy to configure and maintain


The init process is intended to run on top of Linux and makes use of some
Linux specific features (e.g. signalfd), but if sufficient interest exists,
it should still be possible to make it run on some BSDs or whatever else.

The init system tries to mimic the concept of unit files from systemd as those
were considered to be a good design choice.

Those parameterizeable service description files are stored in `/usr/share/init`
by default. Services are enabled by creating a symlink in `/etc/init.d`. This
can be done using the `service` command line tool.

See [docs/services.md](docs/services.md) for more information on service
description files.

See [docs/bootup.md](docs/bootup.md) for more information on what the init
daemon does during system boot.


Right now, the system is in a "basically works" proof of concept stage and
needs some more work to become usable.

There are plans for *maybe* *eventually* adding support for Linux name
spaces, seccomp filters and cgroups as needed in the medium future.


## Why

There are already a bunch of similar projects out there that have been
considered for use in the Pygos system. The reason for starting a new
one was mainly dissatisfaction with the existing ones. Other Projects
that have been considered include:

- systemd

    Contains a lot of good ideas, but it is HUGE. It has tons of
    dependencies. It implements tons of things that it simply shouldn't.
    It has a horrid, "modern", python based, hipster build system.
    It's simply too damn large and complex.

- SystemV init

    A bad combination of unnecessary complexity where it isn't needed and a
    complete lack of abstraction where it would be needed. Shell script
    copy and paste madness. There are reasons people started developing
    alternatives (other than "hurr-durr-parallel-boots").

- upstart

    Seems nice overall, but needlessly big and complex for the intended
    use case in Pygos. Would have needlessly added D-Bus to the system.

- OpenRC

    Was already integrated into Pygos. Things turned out to be broken.
    Upstream developers did not accept fixes (after ignoring them for weeks
    and preferring typo fixes instead). Complaints from other people who
    tried to contribute fixes were observed on GitHub. Complaints from
    package maintainers about deteriorating code quality were observed
    on the official IRC channel. Documentation is non-existent.

- daemon tools and similar (runnit, s6, minit, ...)

    The sixties are over. And even code from that era is more readable. The
    source code for those projects should better be tossed out the window and
    rewritten from scratch. If you are a first semester CS student and you
    hand something like this in as a homework, the best you might get is a
    well deserved slap on the back of your head.

- busybox init

    Nice and simple. Probably the best fit if the rest of your user space is
    busybox as well.
