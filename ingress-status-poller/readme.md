# Poller

This is a CLI that polls BBC's Notification Ingress API continuously to track
when a downtime occurs.

It was used when moving from x86 architecture to AMD to measure the best way to
implement migation without encountering any downtime.

- `-d`: String indicating poll interval duration, e.g. `2s`.
- `-e`: Cosmos environment, one of `int`, `test` or `live`.
