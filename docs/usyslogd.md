# Syslogd Implementation

A tiny syslogd implementation `usyslogd` is provided as part of this package.

It opens a socket in `/dev/log`, processes syslog messages and forwards the
parsed message to a modular backend interface.

Currently, there is only one implementation of the backend interface that dumps
the log messages into files in the processes working directory (by default
`/var/log`).

A simple log rotation scheme has been implemented.


## Security Considerations

By default, the daemon switches its working directory to `/var/log`. The
directory is created if it doesn't exist and the daemon always tries to
change its mode to one that doesn't allow other users (except group members)
to access the directory.

If told to so on the command line, the daemon chroots to the log directory.

By default, the daemon then tries to drop privileges by switching to user and
group named `syslogd` if they exist (any other user or group can be specified
on the command line; doing so causes syslogd to fail if they don't exist).


On a system that hosts accounts for multiple users that may be more or less
trusted, one may consider only giving system services access to the syslog
socket and not allowing regular users. Otherwise, a user may flood the syslog
daemon with messages, possibly leading to resource starvation, or (in the case
of size limited log rotation outlined below) to the loss of otherwise critical
log messages. Since this is not the primary target of the Pygos system, such
a mechanism is not yet implemented.

In case of a system where only daemons are running, the above mentioned
security measure is useless. If a remote attacker manages to get regular user
privileges, you already have a different, much greater problem. Also, a remote
attacker would have to compromise a local daemon that already has special
access to the syslog socket, which is again your least concern in this
scenario.


## Logrotation

The backend can be configured to do log rotation in a continuous fashion (i.e.
in a way that log messages aren't lost), or in a way where it drops old
messages. Furthermore, the backend can be configured to automatically do a log
rotation if a certain size threshold is hit.

If the `usyslogd` receives a `SIGHUP`, it tells the backend to do log rotation.

In the case of the size threshold, the backend is expected to do the rotation
on its own if the predetermined limit is hit.


## File Based Backend

The file based backend writes log messages to files in the current working
directory (by default `/var/log`), named either after the ident string (if
specified) or the facility name.

Log messages are prefixed with an ISO 8601 time stamp, optionally the facility
name (unless part of the file name), the log level and the senders PID. Each
of those fields is enclosed in brackets.

Log rotation in a continuous fashion means renaming the existing log file to
one suffixed with the current time stamp. Overwriting old messages renaming
the log file by appending a constant `.1` suffix.


## Default Configuration

The default service configuration limits the log file size to 8 KiB and
configures the daemon to overwrite old messages when rotating log files,
effectively limiting the amount of log data to 16 KiB per source or facility.

The intended use case in the Pygos system is logging to a ramdisk without
exhausting available memory.


## Possible Future Directions

In the near term future, the daemon probably requires more fine grained control
over logging such as setting a minimum log level or a way to configure limits
per facility or service.

In the medium term future, extended resource control using c-groups might be
a possibility.

Future directions may include adding other backends, such as forwarding the
log messages to a central server, for instance using syslog over UDP/TCP or
using the front end of some time series database.
