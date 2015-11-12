# codeline-lntcp
Model Railroad Control Point codeline - implemented for Stefan Bormann's Loconet over TCP framework

A codeline message is an 8-byte array of bits, with each bit representing some part of the field appliance,
such as a Maintainer Call light, a Switch-Normal or Signal-at-Stop indication.

Codelines carry control and indication messages, 
* controls usually are from a dispatcher to a particular field unit, while 
* indications are from a field unit back to the dispatcher.

code2LN converts a codeline message into a Loconet OPC_PEER_XFER message
LN2code converts a OPC_PEER_XFER message into a codeline message
