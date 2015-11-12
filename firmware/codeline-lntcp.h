#ifndef _CODELINE_LNTCP_
#define _CODELINE_LNTCP_

// Note: Because this code "abuses" the pxct1 byte used by SV2,
// no SV2-style devices can be allowed on this loconet.

// SV1 must have the upper nibbles of pxct1 = 0x0x and pxct2 = 0x1x
// SV2 must have the upper nibbles of pxct1 = 0x1x and pxct2 = 0x1x

// Use a reserved bit from pxct1 to indicate packets that we routed, 
// so we ignore them when reflected/retransmitted


class codeline_lntcp {
public:
	enum { ROUTE = 0, IGNORE=1, SEEN=2, PXCT1_RESERVED = 0x20, OPC_PEER_XFER = 0xE5 };
	
	codeline_lntcp(byte *server, unsigned long port) {
		_server[0] = server[0];
		_server[1] = server[1];
		_server[2] = server[2];
		_server[3] = server[3];
		_port      = port;
		_connected = 0;
	};
	void stop(void) {
		if (_connected) {
			_client.stop();
			_connected = 0;
		}
	}
	int  connect() {
		if (!_connected) {
		    _connected = _client.connect(_server, _port);
		}
		return _connected;
	};
	int  connected() {
		return _connected;
	};
	int available(void) {
		if (_connected) return _client.available();
		return 0;
	};
	char read(void) {
		if (_connected) return _client.read();
		return 0;
	};
	void send(const char *payload) {
	    if (_connected) _client.print(payload);
	};
	void code2LN(char *s, int *code, int src, int dst);
	int  LN2code(char *s, int *code, int *opcode, int *src, int *dst);

private:
	byte          _server[4];
	unsigned long _port;
	TCPClient     _client;
	int           _connected;
	unsigned      getValue(char *s);
};
	
#endif
