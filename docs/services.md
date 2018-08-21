# Service Files

The init process reads service descriptions from `/etc/init.d` which usually
contains symlinks to the actual service files which can be conveniently removed
or added to disable or enable services.

Default services provided by this package are installed in `/usr/share/init`,
i.e. this is where the symlinks point to.


Note that the actual locations may be different, depending on the configure
flags used.


Service descriptions can be parameterized. The arguments are extracted from the
name of the symlink. Currently only 1 parameter is supported. The argument
value is separated from the service name by an '@' character in the symlink
name.

The file name of the sysmlink, excluding any parameters, is accepted as the
canonical name of a service and used when referring to it via command line
utilities or when injecting dependencies from a service file.


## Syntax

Each line in a service description is made up of a keyword, followed by one
or more arguments and terminated by a line break.

Blank lines are ignored and shell-style comments can be used.

Arguments are separated by space. Quotation marks can be used to treat
something containing spaces or comment character as a single argument.

In between quotation marks, C-style escape sequences can be used.

Argument substitution (arguments derived from the symlink name) can be
done using a '%' sign, followed by the argument index. A '%' sign can be
escaped by writing '%%'.


## Targets and Types

Service files specify a *target* which is basically like a SystemV runlevel
and can be one of the following:

* boot
* reboot
* shutdown

After parsing the configuration files, the init process starts running the
services for the `boot` target in a topological order as determined by their
dependencies.

Services can be of one of the following *types*:

* wait
* once
* respawn

Services of type `wait` are started exactly once and the init process waits
until they terminate before continuing with other services.

The type `once` also only runs services once, but immediately continues
starting other services in the mean time without waiting. The init process
only waits for `once` types when transitioning to another target.

Services of type `respawn` also don't stall the init process and are re-started
whenever they terminate.

The keyword `limit` can be used a after `respawn` to specify how often a service
may be restarted before giving up.


## Dependencies

A service description file can specify dependencies using the keywords `after`
and `before`, each followed by a space separated list of service names.

The init program executes a service after all the services specified by the
`after` keyword have been started (type once or respawn) or have been completed
(type wait).

The `before` keyword injects dependencies in reverse, i.e. all services
specified after the `before` keyword are only executed after the service in
question has been started.

If a service specified by `after` or `before` does not exist, it is simply
ignored. This can occur for instance if the specified service is not enabled
at all in the current configuration.


## Running Services

If a service contains an `exec` line, the init process attempts to run it
using the `runsvc` helper program that sets up the environment, attempts to
execute the specified command line and passes the exit status back to the init
process.

If multiple exec lines are specified, `runsvc` executes them sequentially and
stops if any one returns a non-zero exit status.

The environment variables visible to the service processes are read
from `/etc/initd.env`.

If the service description contains a `tty` field, the specified device file
is opened by runsvc and standard I/O is redirected to it and a new session
is created. The keyword `truncate` can be used to make `runsvc` truncate the
file to zero size first.

For convenience, multiple exec lines can be wrapped into braces, as can be
seen in one of the examples below.


## Example

Below is an annotated example for a simple, service description for a
generic, parameterized getty service:

    #
    # The text that init should print out when the status of the
    # service changes.
    #
    # The '%0' is replaced with the first argument extracted from the
    # symlink name.
    #
    description "agetty on %0"

    # Restart the getty when it terminates.
    type respawn

    # We want to spawn gettys when booting the system
    target boot

    # Only run this service after the 'sysinit' service is done
    after sysinit

    #
    # Redirect all I/O of the process to this file. The specified device file
	# is used as a controlling tty for the process and a new session is created
	# with the service process as session leader.
    #
    # In this example, we derive the controlling tty from the service
    # description argument.
    #
    tty "/dev/%0"

    # In order to run the service, simply fire up the agetty program
    exec agetty %0 linux

If a service should sequentially run multiple commands, they can be grouped
inside braces as can be seen in the following, abbreviated example:

    description "mount /var"
    type wait
    target boot
    before vfs
    exec {
        mount -t tmpfs none /var
        mkdir /var/log -m 0755
        mkdir /var/spool -m 0755
        mkdir /var/lib -m 0755
        mkdir /var/tmp -m 0755
        mount --bind /cfg/preserve/var_lib /var/lib
    }
