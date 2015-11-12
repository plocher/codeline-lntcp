#ifndef _CODELINE_LNET_
#define _CODELINE_LNET_

// Note: Because this code "abuses" the pxct1 byte used by SV2,
// no SV2-style devices can be allowed on this loconet.

// SV1 must have the upper nibbles of pxct1 = 0x0x and pxct2 = 0x1x
// SV2 must have the upper nibbles of pxct1 = 0x1x and pxct2 = 0x1x

// Use a reserved bit from pxct1 to indicate packets that we routed, 
// so we ignore them when reflected/retransmitted


class codeline_lntcp {
public:
	enum { ROUTE = 0, IGNORE=1, SEEN=2, PXCT1_RESERVED = 0x20, OPC_PEER_XFER = 0xE5 };

	void code2LN(char *s, int *code, int src, int dst);
	int  LN2code(char *s, int *code, int *opcode, int *src, int *dst);

#endif
