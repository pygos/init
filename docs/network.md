# Static Network Configuration

The default configuration provides multiple services that perform network
initialization and static configuration using helper scripts that require
programs from the `iproute2` package.

Configuration files are typically stored in `/etc/netcfg/` (depending on
configure options).

Please note that the loopback device is treated specially and not included in
any of the network configuration outlined below. The loopback device is brought
up and configured by a dedicated service long before the network configuration
is done.


## Interface Renaming

If the `ifrename` service is enabled (it is disabled by default), network
interfaces are renamed based on a rule set stored in the file `ifrename`.
The file contains comma separated shell globing patterns for the current
interface name, MAC address and a prefix for the new interface name.

For each network interface, rules are processed top to bottom. If the first two
globing patterns apply, the interface is renamed. Interfaces with the same
prefix are sorted by mac address and a running index is appended to the prefix.

If none of the rules apply, the interface name is left unchanged.


The intent is, to provide a way to configure persistent, deterministic names for
at least all network interfaces that are permanently installed on a board.

Extension cards or external network adapters should be given a different prefix
to avoid changes in the order as they come and go.


## Interface Configuration

After interface renaming, for each network interface, the configuration path is
scanned for files with the same name as the interface.

Each successfully found configuration file is processed line by line, top to
bottom. Each line may contain a keyword, followed by multiple arguments.

The following keywords can be used to add IPv4 or IPv6 network addresses to
an interface:

 * address
 * addr
 * ip
 * ip6
 * ipv6

Those commands are expected to be followed by an IPv4 or IPv6 address and
network mask.


Furthermore, the following commands can be used for configuring interface
parameters:

 * `arp {on|off}`
 * `multicast {on|off}`
 * `mtu <value>`
 * `offload [rx {on|off}] [tx {on|off}] [sg {on|off}] [tso {on|off}]`
 * `offload [gso {on|off}] [gro {on|off}] [lro {on|off}] [rxvlan {on|off}]`
 * `offload [txvlan {on|off}] [ntuple {on|off}] [rxhash {on|off}]`
 * `offload [ufo {on|off}]`


## Route Configuration

After interface configuration is done, routes and rules are restored from a
file named `routes` in the same configuration path.

The file may contain lines starting with `route` or `rule`. Everything that
follows is passed on to `ip route add` or `ip rule add` respectively.


## Net Filter Tables


An additional service is provided that restores the nft rule set from
`/etc/nftables.rules`.
