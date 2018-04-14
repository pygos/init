# Service Files

Services that can be started and managed by init are described by service
description files stored in `/usr/share/init`.

The init process actually reads from `/etc/init.d` which contains symlinks to
the actual service files.

Enabling a service means adding a symlink, disabling means removing a symlink.

Service descriptions can be parameterized. The arguments are extracted from the
name of the symlink. Currently only 1 parameter is supported. The argument
value is separated from the service name by an '@' character in the symlink
name.



Below is an annotated example for a simple, service description for a
generic, parameterized agetty service:

    #
    # The text that init should print out when the status of the
    # service changes.
    #
    # The '%0' is replaced with the first argument extracted from the
    # symlink name.
    #
    description "agetty on %0"

    #
    # How to run the service. 'respawn' means restart the service when it
    # terminates, 'once' means run it only once and continue with other
    # services in the mean while, 'wait' means run it once, but block until
    # it exits.
    #
    type respawn

    #
    # When to start the service. 'boot' means when booting the system. Other
    # options are 'reboot', 'shutdown' and 'ctrlaltdel'. The system always
    # starts into the 'boot' target and then later transitions to one of the
    # others.
    #
    target boot

    #
    # A list of service names that must be started before this service can
    # be run, i.e. this services needs to be started after those.
    #
    # This can only refer to generic names, not specific instances. For
    # instance, you can say "after getty" to make sure a service comes up after
    # all gettys are started, but you cannot specify "after agetty@tty1".
    #
    # Similar to 'after', there is also a 'before' keyword for specifying
    # dependencies.
    #
    after sysinit

    #
    # The 'tty' directive specifies a file to which all I/O of the process is
    # redirected. The specified device file is used as a controlling tty for
    # the process and a new session is created with the service process as
    # session leader.
    #
    # In this example, we derive the controlling tty from the service
    # description argument.
    #
    tty "/dev/%0"
    
    #
    # The 'exec' directive specifies the command to execute in order to start
    # the service. See in the example below on how to run multiple commands.
    #
    # Again we use the argument to specify what terminal our getty
    # should run on.
    #
    exec agetty %0 linux

As can be seen in this simple example, each line in a service description is
made up of a keyword, followed by one or more arguments and terminated by a
line break.

Blank lines are ignored and shell-style comments can be used.

Arguments are separated by space. Quotation marks can be used to treat
something containing spaces or comment character as a single argument.

In between quotation marks, C-style escape sequences can be used.

Argument substitution (arguments derived from the symlink name) can be
done using a '%' sign, followed by the argument index. A '%' sign can be
escaped by writing '%%'.


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

