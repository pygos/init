# About

This directory contains the source code for a tiny service supervision
framework devised for the Pygos system, consisting of an init daemon,
a _definitely_ non standards compliant cron implementation and various
command line utilities.


The programs of this package are developed first and foremost for GNU/Linux
systems, so there are some GNU and some Linux extensions used and some of the
code may unintentionally rely on Linux specific behavior.

Nevertheless, if sufficient interest exists, it should be possible to make it
run on BSDs or other Unix-like systems, but some effort may be required.


The init system tries to mimic the concept of unit files from systemd as those
were considered to be a good design choice.

In a typical setup, the parameterizeable service description files are stored
in `/usr/share/init` by default. Services are enabled by creating a symlink
in `/etc/init.d`. This can be done more conveniently using the `service`
command line tool.


A default setup for the Pygos system, including helper scripts for setting up
mount points and for network configuration, is provided in a seperate package.
This package only contains the bare init framework without any default
configuration.


Right now, the system is in a "basically works" proof of concept stage and
needs some more work to become usable.

There are plans for *maybe* *eventually* adding more fancy features like
support for Linux name spaces, seccomp filters and cgroups, but right now,
features are added only when the need arises.


See [docs/init.md](docs/init.md) for more information on the design,
implementation and caveats of the init daemon.

See [docs/cmdline.md](docs/cmdline.md) for an explanation on the available
command line tools.

See [docs/services.md](docs/services.md) for more information on service
description files.

See [docs/gcron.md](docs/gcron.md) for details on the cron implementation.


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
