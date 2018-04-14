# System Bootup Process

## Initial Ram Disk to Rootfs transition

After mounting the root filesystem, either the kernel or the initial ram disk
startup process is expected to exec the init program from the root filesystem.

At the current time, there is no support for re-scanning the service files
*yet*, so when init is started, the final configuration in `/etc/init.d` has to
be present. As a result, we currently cannot perform mounting of `/etc/` or
packing init into the initial ram disk and doing the rootfs transition.

Also, as a result of this, changing the service configuration requires a system
reboot to be effective.

This _will_ change in the future.


## Processing Service Descriptions

The init process reads service description files from `/etc/init.d` which are
usually symlinks to actual files in `/usr/share/init`.

The exact locations may be changed through configure flags when compiling init.

Service files specify a *target* which is basically like a SystemV runlevel and
can be one of the following:

* boot
* reboot
* shutdown
* ctrlaltdel

After parsing the configuration files, the init process starts running the
services for the `boot` target in a topological order as determined by their
*before* and *after* dependencies.

Services can be of one of the following *types*:

* wait
* once
* respawn

Services of type `wait` are started exactly once and the init process waits
until they terminate before continuing with other services.

The type `once` also only runs services once, but immediately continues
starting other services in the mean time without waiting.

Services of type `respawn` also don't stall the init process and are re-started
whenever they terminate.

## Service Process Setup

If a service description contains only a single `exec` line, the init process
forks and then execs the command directly in the child process.

If the service description contains a `tty` field, the specified device file
is opened in the child process and standard I/O is redirected to it before
calling exec. Also, a new session is created.


If a service description contains multiple `exec` lines, the init process forks
off to a single child process that does the same setup as above, and then runs
the command lines sequentially by forking a second time for each one, followed
by an exec in the grand child and a wait in the original child.

If a single command line returns something other than `EXIT_SUCCESS`,
processing of multiple command lines is immediately stopped and the offending
exit status is returned to init.

