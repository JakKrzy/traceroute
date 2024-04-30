# Traceroute

This program traces packet's route to given address.
It works in steps: in each step TTL is increased and three packets are sent to given address.

## Usage
`make` then `sudo ./traceroute X.X.X.X` where X.X.X.X is the target address.

## Output
`*` - no responses were received in this step,
`X.X.X.X ???` - not all responses were received in this step,
`X.X.X.X Y ms` - this address responded three times with average response time of Y milliseconds.


