# The Init Daemon

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


## Service Types and Targets

A service file has a *type*, specifying whether the service should be run once
or restarted when it terminates and a *target*, specifying when the service
should be run.

The *target* is similar to a runlevel in System V init. The init daemon
currently knows about the following targets:

* boot
* reboot
* shutdown

When `init` is run, it starts all the services for the `boot` target. From the
`boot` target it can transition to any other target and execute the services
for the specified target.

The `reboot` and `shutdown` targets cannot transition to any other target and
when invoked, cause initd to drop everything else it intended to do.

For the `reboot` and `shutdown` targets, respawn type processes are no longer
restarted when they terminate and once all services have been executed, the
`init` program performs a hard system reboot or power off.

The init program tries to capture the `CTRL+ALT+DELETE` key sequence (or its
local equivalent) and transitions to the reboot target if pressed.


## Service Configuration Rescan

TBD


## Control Socket and Signals

The `init` program catches the following signals:

* `SIGCHLD`
* `SIGINT`
* `SIGTERM`

The `SIGCHLD` handler implements standard process reaping. If a terminated
process belongs to one of the supervised services, the configured action is
taken (e.g. restarting it).

When `SIGINT` is caugth, `init` transitions to the `reboot` target. Similarly,
`SIGTERM` causes `init` to transition to the `shutdown` target.


For more complex tasks, `init` creates a control socket that the command line
tools included in this package can use. For the time being, the control socket
can only tell the init daemon to transition to the `reboot` or `shutdown`
target.
