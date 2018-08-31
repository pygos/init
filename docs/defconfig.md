# Default Service Configuration

## Pseudo Services

The default configuration contains a number of "pseudo services" in the boot
target that don't actually do anything but are merely used as anchors in
service dependencies, i.e. they indicate that some sort of milestone in the
boot sequence has been reached. Everything that is part of that milestone
specifies that it should be run *before* that pseudo service and everything
that requires that this milestone has been reached, specifies that it wants
to run afterwards.

The pseudo targets are (in the order that they are executed):

 * vfs

   All services that do mount point setup go before this, all service that
   depend on the fully mounted rootfs go after this.

 * sysinit

   The system has reached a sane state, i.e. the hostname is set, the system
   clock has a sane value, modules and kernel parameters are loaded, some
   very basic, fundamental services are running (e.g. syslog).
   Everything that is part of that setup process goes between `vfs` and
   `sysinit`, everything that requires a sane setup goes *after* `sysinit`.

 * network

   Network configuration is done. All services that do network configuration
   should position themselves between `sysinit` and `network`. Everything that
   requires a fully configured networking setup should go *after* `network`.

## Default Bootup Services

This section outlines the services for the boot target that are enabled by
default.


The following services are enabled by default and run *before* the `vfs` target
for filesystem setup:

 * procfs - mount `procfs` to `/proc` and try to mount additional pseudo
   filesystems in `/proc` such as `binfmt_misc`
 * tmpfs - mount a `tmpfs` to `/tmp`
 * sysfs - mount `sysfs` to `/sys` and try to mount additional pseudo
   filesystems in `/sys` (e.g. `securityfs`, `configfs`, ...)
 * devfs - mount `devtmpfs` to `/dev`, try to mount additional pseudo
   filesystems in `/dev` (e.g. `devpts`, `mqueue`, ...) and try to create
   some additional device nodes and symlinks.


The following services are enabled by default and configured to run *after*
the `vfs` target and *before* the `sysinit` target:

 * hostname - reload hostname `/etc/hostname`
 * loopback - bring the loopback device up
 * usyslogd - if the `usyslogd` services is compiled with this package, this
   service is enabled by default and starts `usyslogd`.
 * modules - iterate over the file `/etc/modules` and try to load each module
   using modprobe.
 * sysctl - restore kernel parameters using `sysctl --system`. See `sysctl(8)`
   for a list of possible locations that the parameters are read from.


The following services are enabled by default and configured to run *after*
the `sysinit` target and *before* the `network` target:

 * ifcfg - static network configuration
   Does the static network configuration outlined in [network.md](network.md)


## Default Shutdown and Reboot Services

For the shutdown and reboot targets, the following services are executed:

 * sigterm - send the SIGTERM signal to all processes and wait for 5 seconds
 * sigkill - send the SIGKILL signal to all remaining processes
 * ifdown - bring all network interfaces down
 * sync - run the sync command


## Additional Services not Enabled by Default

 * agetty - A parameterizeable, respawn type `agetty` service. The first
   parameter is the terminal device that the getty should run on.
 * dhcpcdmaster - If one or more network interfaces should be configured using
   dhcpcd, this service starts a central `dhcpcd` master instance.
 * dhcpcd - A parameterizeable single shot service that signals the `dhcpcd`
   master that it should configure a specific interface. The first parameter
   is the interface that should be configured by `dhcpcd`.
 * dnsmasq - A respawn type service for the `dnsmasq` DNS and DHCP server.
 * hostapd - If the system should operate a WIFI access point, this respawn
   type service can be enabled to manage an instace of the `hostapd` program.
 * unbound - A respawn type service that manages an instance of the `unbound`
   name resolver.
 * usyslogd - A respawn type service that manages an instance of the `usyslogd`
   syslogd implementation that is part of this package.
 * hwclock - If the system has a hardware clock, this service can restore the
   kernels clock from the hardware at bootup, between the `vfs` and `sysinit`
   targets.
 * nft - If enabled, restores net filter table rules during boot.
 * swclock - For systems that don't have a hardware clock, this service
   restores a somewhat usable time from a file during boot.
 * swclocksave - For systems that don't have a hardware clock, this service
   saves the current time to a file during shutdown or reboot.
 * sshd_keygen - A wait type service that generates host keys for the OpenSSH
   server and then disables itself.
 * sshd - Starts an OpenSSH server after the network pseudo service and after
   the sshd_keygen service.
