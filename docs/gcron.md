# Gcron

Gcron is a small daemon that executes batch commands once a certain
condition is met.

In a typical installation, it reads configuration files from `/etc/gcron.d`.
If used together with the init system in this package, the `service` command
can be used to administer symlinks in that directory, pointing
to `/usr/share/init/<name>.gcron`.

Each file in the configuration directory represents a single scheduled batch
job. The syntax and most of the keywords are similar to `initd` service files
(See [services.md](services.md)).

## Cron Style Patterns

The following keywords can be used to specify classic cron style patterns for
when a job should be run:

 * `hour`
 * `minute`
 * `dayofmonth`
 * `dayofweek`
 * `month`

For each of those keywords, a comma separated sequence of times can be
specified. Time ranges can be specified using the syntax `<start>-<end>`,
or using `*` for every possible value. A sequence (either range or star)
can be suffixed with `/<step>` to specify an increment.
For instance, `minute */5` means every five minutes and `minute 15-30/2`
means every two minutes between quarter past and half past.

In addition to numeric values, the keywords `dayofweek` and `month` allow
specifying 3 letter, uppercase week day and moth names such as `MON`, `TUE`,
etc and `JAN`, `FEB`, ...

The job is only run when all specified conditions are met. Omitting a field
is the same as specifying `*`.

## Named Intervals

Alternatively to the above, the keyword `interval` can be used. The following
intervals can be specified:

 * `yearly` or `annually` means on every January the first at midnight.
 * `monthly` means on every first of the month at midnight.
 * `weekly` means every Sunday at midnight.
 * `daily` means every day at midnight.
 * `hourly` means every first minute of the hour.

## Command Specification

To specify *what* should be done once the condition is met, the following
keywords can be used:

 * `exec` - the command to run. Multiple commands can be grouped
   using curly braces.
 * `user` - a user name or ID to set before running the commands.
 * `group` - a group name or ID to set before running the commands.
 * `tty` - similar to init service files, the controlling tty or output file
   for the batch commands. Like init service files, the `truncate` keyword
   can be used.
