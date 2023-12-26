# TBI v1 - Telemetry Binary Interface version 1

The TBI project implements a versatile and robust telemetry protocol optimized for network usage. The protocol
is designed mainly for use by large fleets of Linux based IoT-devices with cellular, or other pay-as-you-go forms
of connectivity, where the cost of communication is a limiting factor. The protocol supports both one-off real-time
measurements or alerts, and (delta compressed) bundles of history data.

This repository contains the TBI core library implementation, as well as example client and server implementations, and utility scripts to convert the human-readable custom telemetry message specifications to the binary format understood by the library.

## Operation principle
The TBI protocol starts with a normal TCP handshake, followed by the protocol-specific handshake, where the client and server version compatibility is checked, and the client protocol manifest is sent to the server. The protocol manifest contains the versioned data structure definition of up to 16 different telemetry message formats, that the client is expected to send during the session (TODO: just checksum and version?). The server ensures it has the same data structure definition, and either acknowledges the handsake, or ends the handshake.

Example manifest:
```
------------------------------------------------------------------------------------------------------------- - -  -  -
| <TBI magic> | <protocol version> | <start epoch timestamp> | <data structure version> | no. of structures | <1..16 TLV-like structures>
------------------------------------------------------------------------------------------------------------- - -  -  -
| 3 bytes     | 1 byte             | x bytes                 | 1 byte

where TLV-like structure definition:
-------------------------------------------------------------------
| structure ID | structure length | <0..N structure items (byte)> |
-------------------------------------------------------------------
| 1 byte       | 1 byte           | N bytes

where structure items is a nibble-size enumeration type, with the following possible values:
--------------------
Type ID | data type 
--------------------
0       | timediff s
1       | timediff ms
2       | uint8
3       | int8
4       | uint16
5       | int16
6       | uint32
7       | int32
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
----------------------------------------------------------------------------------------------
| no. of values in current format | format spec len | <format spec> | stuffing      | <data> |
----------------------------------------------------------------------------------------------
| 1 byte                          | 1 byte          | N bytes       | byte align    | N bytes

where format spec is:
---------------------------------------------------------------------
| no. of bits for struct member 0 | no. of bits for struct member 1 | ...  
---------------------------------------------------------------------
| 6 bits                          | 6 bits

and finally the data, represented in the number of bits as defined by format spec
------------------------------------------------------------- - -  -  -
| struct member 0 | struct member 1 | ... | struct member N | struct member 0 ...
------------------------------------------------------------- - -  -  -
| 0-32 bits       | 0-32 bits       | ... | 0-32 bits       | 0-32 bits
```

The DCB frame format may be changed mid-frame with a new definition. This allows for representing non-changing periods of time series data very efficiently, with an entire data structure represented by only the time difference, or even 0 bits, if timestamp is not a member of the data. The TBI frame constructor automatically chooses the frame formats to send the data in least number of bits

## Building
To build the library and test clients, perform the following commands:

```
mkdir build && cd build
cmake ..
make
```

## Running
The example client and server can be found under the ```bin/``` directory