/*
 * Loconet over TCP to Codeline
 */

#include "application.h"
#include "codeline_lntcp/codeline_lntcp.h"

typedef struct peerxfer_t {
    uint8_t command;
    uint8_t mesg_size;     /* ummmmm, size of the message in bytes?                */
    uint8_t src;           /* source of transfer                                   */
    uint8_t dst_l;         /* ls 7 bits of destination                             */
    uint8_t dst_h;         /* ms 7 bits of destination                             */
    uint8_t pxct1;
    uint8_t d1;            /* data byte 1                                          */
    uint8_t d2;            /* data byte 2                                          */
    uint8_t d3;            /* data byte 3                                          */
    uint8_t d4;            /* data byte 4                                          */
    uint8_t pxct2;
    uint8_t d5;            /* data byte 5                                          */
    uint8_t d6;            /* data byte 6                                          */
    uint8_t d7;            /* data byte 7                                          */
    uint8_t d8;            /* data byte 8                                          */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} peerXferMsg;

// SV1 must have the upper nibbles of pxct1 = 0x0x and pxct2 = 0x1x
// SV2 must have the upper nibbles of pxct1 = 0x1x and pxct2 = 0x1x

// Use a reserved bit from pxct1 to indicate packets that we routed, 
// so we ignore them when reflected/retransmitted

// SPCoast codeline uses ONLY SV1 format packets, SO WE CAN "REUSE" THIS RESERVED BIT.
// This DOES mean that no SV2-style devices can be allowed on this loconet.

void codeline_lntcp::code2LN(char *s, int *code, int src, int dst) {
    char data[17];
	data[ 0 ] = codeline_lntcp::OPC_PEER_XFER ;
	data[ 1 ] = 0x10;               // packet length = 16
	data[ 2 ] = (src & 0x7F);       // SRC
	data[ 3 ] = (dst & 0x7F);       // DSTL
	data[ 4 ] = 0x00;               // must be 0x01 - 00=JMP debug

	int pxct = codeline_lntcp::PXCT1_RESERVED;  
	if (code[0] & 0x80) pxct |= 0b0001;
	if (code[1] & 0x80) pxct |= 0b0010;
	if (code[2] & 0x80) pxct |= 0b0100;
	if (code[3] & 0x80) pxct |= 0b1000;

	data[ 5 ] =  pxct;    // pxct1
	data[ 6 ] = code[0] & 0x7F;  
	data[ 7 ] = code[1] & 0x7F;  
	data[ 8 ] = code[2] & 0x7F;  
	data[ 9 ] = code[3] & 0x7F;  

	pxct = 0x10;
	if (code[4] & 0x80) pxct |= 0b0001;
	if (code[5] & 0x80) pxct |= 0b0010;
	if (code[6] & 0x80) pxct |= 0b0100;
	if (code[7] & 0x80) pxct |= 0b1000;

	data[ 10 ] = pxct;    // pxct2
	data[ 11 ] = code[4] & 0x7F;  
	data[ 12 ] = code[5] & 0x7F;  
	data[ 13 ] = code[6] & 0x7F;  
	data[ 14 ] = code[7] & 0x7F;   
	 
    byte checksum = 0xFF; 
	for (int i = 0; i < 15; i++) {
      checksum ^= data[i];
    }
    data[ 15 ] = checksum; //checksum
    
    sprintf(s, "SEND %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",  
            data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
}


// convenience wrapper arround strtok() - call first time with a string, the rest with NULL
unsigned codeline_lntcp::getValue(char *s) {
    static char buffer4strtok[80];
    char* myCopy;
    if (s) {
        strcpy(buffer4strtok, s);
        myCopy = strtok(buffer4strtok, " ");
    } else {
        myCopy = strtok(NULL, " ");
    }
    return strtoul(myCopy, 0, 16);
}

// Parse a LoconetOverTCP string into components,
// grab 8 data bytes and load into code array
// This is probably better done with sscanf()
int codeline_lntcp::LN2code(char *s, int *code, int *opcode, int *src, int *dst) {
    peerXferMsg pkt;
    *opcode  = getValue(s);
    
    if (*opcode == codeline_lntcp::OPC_PEER_XFER) {
        pkt.command   = *opcode;
        pkt.mesg_size = getValue(NULL);
        pkt.src       = getValue(NULL);     // source of transfer (0x00-0x7F)
        pkt.dst_l     = getValue(NULL);     // destination  (0x00-0x7F)
        pkt.dst_h     = getValue(NULL);     // must be 0x01
        pkt.pxct1     = getValue(NULL);     //                           
        pkt.d1        = getValue(NULL);     // data byte 1 
        pkt.d2        = getValue(NULL);     // data byte 2
        pkt.d3        = getValue(NULL);     // data byte 3
        pkt.d4        = getValue(NULL);     // data byte 4
        pkt.pxct2     = getValue(NULL);
        pkt.d5        = getValue(NULL);     // data byte 5 
        pkt.d6        = getValue(NULL);     // data byte 6 
        pkt.d7        = getValue(NULL);     // data byte 7
        pkt.d8        = getValue(NULL);     // data byte 8
        pkt.chksum    = getValue(NULL);     // exclusive-or checksum for the message

        *src = (byte)pkt.src   & 0x7f;
	    *dst = (byte)pkt.dst_l & 0x7f;
        code[0] = (byte)pkt.d1;  
        code[1] = (byte)pkt.d2;  
        code[2] = (byte)pkt.d3;
        code[3] = (byte)pkt.d4;
        code[4] = (byte)pkt.d5;  
        code[5] = (byte)pkt.d6;  
        code[6] = (byte)pkt.d7;
        code[7] = (byte)pkt.d8;
        if ((byte)pkt.pxct1 & 0b00000001) code[0] |= 0b10000000;
        if ((byte)pkt.pxct1 & 0b00000010) code[1] |= 0b10000000;
        if ((byte)pkt.pxct1 & 0b00000100) code[2] |= 0b10000000;
        if ((byte)pkt.pxct1 & 0b00001000) code[3] |= 0b10000000;
     
        if ((byte)pkt.pxct2 & 0b00000001) code[4] |= 0b10000000;
        if ((byte)pkt.pxct2 & 0b00000010) code[5] |= 0b10000000;
        if ((byte)pkt.pxct2 & 0b00000100) code[6] |= 0b10000000;
        if ((byte)pkt.pxct2 & 0b00001000) code[7] |= 0b10000000;
        
        if (((byte)pkt.pxct1 & PXCT1_RESERVED) == PXCT1_RESERVED) { // If the "don't route" bit is set...
            // SV1 must have the upper nibbles of pxct1 = 0x0x and pxct2 = 0x1x
            // FORCE all packets to look like SV1
            pkt.pxct1 = (pkt.pxct1 & 0X0F) | 0X00;
            pkt.pxct2 = (pkt.pxct2 & 0X0F) | 0X10;
            return SEEN;
        } else {
            return ROUTE;
        }
    }
    return IGNORE;
}

