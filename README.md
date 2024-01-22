***Work in progress! Feel free to try it out, but don't use for anything serious :)***

# TBI v1 - Telemetry Binary Interface version 1

The TBI project implements a versatile and robust telemetry protocol optimized for network usage. The protocol
is designed mainly for use by large fleets of Linux based IoT-devices with cellular, or other pay-as-you-go forms
of connectivity, where the cost of communication is a limiting factor. The protocol supports both one-off real-time
measurements or alerts, and (delta compressed) bundles of history data.

This repository contains the TBI core library implementation, as well as example client and server implementations, and utility scripts to convert the human-readable custom telemetry message specifications to the binary format understood by the library.

## Project status

**Implemented:**
* Message spec code generation from JSON
* Sending RTM messages (client)
* Receiving RTM messages from a single client (server)
* Example client and server

**To be implemented:**
* Sending/receiving DCB messages
* DCB message compression
* Multi-client support on server
* Command line parameters or configuration file
* Thread-safety
* TLS

## Operation principle
The TBI protocol starts with a normal TCP handshake, followed by the protocol-specific handshake, where the client and server version compatibility is checked. The handshake includes the TBI protocol version, message schema version and a checksum of its machine-understandable representation, as well as the client timestamp. The server ensures it has the same message schema version, and either acknowledges the handshake request, or closes the connection.

Client handshake request:
```
------------------------------------------------------------------------------------------------------------
| <TBI magic> | <protocol version> | <start epoch timestamp> | <msg schema version>  | CRC16 of msg schema | 
------------------------------------------------------------------------------------------------------------
| 3 bytes     | 1 byte             | 8 bytes                 | 1 byte                | 2 bytes

```
Server handshake acknowledge:
```
------------------------------------
| <TBI magic> | <protocol version> |
------------------------------------
| 3 bytes     | 1 byte             

```

Once the handshake has been completed, the client and server proceed to the 'streaming' mode, where the client can send telemetry in any of the agreed formats. Each message can be sent in one of two frame formats, an RTM (Real-Time Measurement) format, or a DCB (Delta-Compressed Bundle) format. The RTM frame contains the current values for the data it represents in the agreed format, while the DCB frame contains 1..N separate measurements for that message types in a delta-compressed format.

The RTM frame format is as follows:
```
-----------------------------------------
| flags    | struct ID | structure data |
-----------------------------------------
| 1 nibble | 1 nibble  | N bytes
```
The DCB frame format is as follows:
```
The frame begins with an RTM frame representing the initial value of the structure data (flags different)
-------------------------------------------------------------------------
| flags    | struct ID | structure data (initial value) | <bundle data> |
-------------------------------------------------------------------------
| 1 nibble | 1 nibble  | N bytes                        | N bytes

where <bundle data is> 1..N of:
-------------------------------------------------------------
| no. of values in current format | <format spec> |  <data> |
-------------------------------------------------------------
| 1 byte                          | N bytes       | N bytes

where format spec is:
---------------------------------------------------------------------
| no. of bits for struct member 0 | no. of bits for struct member 1 | ...  
---------------------------------------------------------------------
| 6 bits                          | 6 bits

and finally the data, represented in the number of bits as defined by format spec
------------------------------------------------------------------------------ - -  -  -
| diff direction | struct member 0 | struct member 1 | ... | struct member N | diff direction | struct member 0 ...
------------------------------------------------------------------------------ - -  -  -
| N bits         | 0-32 bits       | ... | 0-32 bits       | 0-32 bits
```

The DCB frame format may be changed mid-frame with a new definition. This allows for representing non-changing periods of time series data very efficiently, with an entire data structure represented by only the time difference, or even 0 bits, if timestamp is not a member of the data. The TBI frame constructor automatically chooses the frame formats to send the data in least number of bits

## Building

Before building, create a message spec (see utils/example.json) and compose a specification header file
```
python3 utils/compose.py <path to message spec>
```
This will generate a header file in generated/messagespec.h

To build the library and test clients, perform the following commands:

```
mkdir build && cd build
cmake ..
make
```

## Running
The example client and server can be found under the ```bin/``` directory