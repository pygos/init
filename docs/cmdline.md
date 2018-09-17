# Available Command Line Tools

## The service command

The `service` utility can be used for control and administration of services.

It is composed of several sub commands run by issuing
`service <command> [arguments..]`.

Currently available service commands are:

 * enable - enable a service with the given list of arguments.
 * disable - disable a service. If the service is parameterized, requires the
   same arguments used for enabling, to disable the specific instance of the
   service.
 * schedule - enable a gcrond service. Only available if this package is built
   with gcrond.
 * unschedule - disnable a gcrond service. Only available if this package is
   built with gcrond.
 * dumpscript - generate an equivalent shell script from the `exec` lines of
   a service after applying all parameter substitutions.
 * list - list all enabled service. A target can be specified to only list
   services for the specified target.
 * help - display a short help text and a list of available commands.


## shutdown and reboot

The programs `shudown` and `reboot` can be used to signal to the `init` program
that it should transition to the `shutdown` or `reboot` target respectively.

The option `-f` or `--force` can be used to by pass the init system entirely
and force a hard reset or power off by directly signalling the kernel.

Running any one of those programs requires superuser privileges.


## syslog

If the `usyslogd` service is built as part of this package, a program called
`syslog` is built that can be used from the command line to send syslog
messages.

This can for instance be used to produce log messages from shell scripts.

The log level, facility and identity string can be specified.
See `syslog --help` for more information.
