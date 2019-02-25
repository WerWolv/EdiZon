function u64(h, l) {       
    if (typeof(h) == 'number' && typeof(l) == 'number') {
        var buffer = new Uint8Array(8);
        for (var i = 0; i < 8; i++) {
            if (i < 4)
                buffer[i] = (l >> 8 * i) & 0xFF;
            else 
                buffer[i] = (h >> 8 * (i - 4)) & 0xFF;
        }
    } else if (h instanceof Uint8Array && l == undefined)
        var buffer = h;
    else if (typeof(h) === 'object') {
        var buffer = new Uint8Array(8);
        for (var i = 0; i < 8; i++)
            buffer[i] = h.buffer[i];
    }
    else return undefined;

    return {
        buffer: buffer,
        add: function(num) {
            var numBuf = new Uint16Array(8);
            if (typeof(num) == 'number') {
                for (var i = 0; i < 4; i++)
                    numBuf[i] = (num >> 8 * i) & 0xFF;
                
                var carry = 0;
                for (var j = 0; j < 8; j++) {
                    numBuf[j] += this.buffer[j] + carry;
                    
                    if (numBuf[j] > 0xFF) {
                        carry = 1;
                        numBuf[j] -= 0x100;
                    } else carry = 0;
                }
            } else if (num instanceof U64) {
                for (var i = 0; i < 8; i++)
                    numBuf[i] = num.buffer[i];
                
                var carry = 0;
                for (var j = 0; j < 8; j++) {
                    numBuf[j] += this.buffer[j] + carry;
                    
                    if (numBuf[j] > 0xFF) {
                        carry = 1;
                        numBuf[j] -= 0x100;
                    } else carry = 0;
                }
            }
            
            for (var k = 0; k < 8; k++)
                this.buffer[k] = numBuf[k]
    
            return this;
        },
        
        sub: function(num) {
            var numBuf = new Int16Array(8);
            if (typeof(num) == 'number') {
                for (var i = 0; i < 4; i++)
                    numBuf[i] = (num >> 8 * i) & 0xFF;
                
                var carry = 0;
                for (var j = 0; j < 8; j++) {
                    numBuf[j] = this.buffer[j] - numBuf[j];
                    numBuf[j] -= carry;
                                    
                    if (numBuf[j] < 0x00) {
                        carry = 1;
                        numBuf[j] += 0x100;
                    } else carry = 0;
                }
            } else if (num instanceof U64) {
                for (var i = 0; i < 8; i++)
                    numBuf[i] = num.buffer[i];
                
                var carry = 0;
                for (var j = 0; j < 8; j++) {
                    numBuf[j] = this.buffer[j] - numBuf[j];
                    numBuf[j] -= carry;
    
                    if (numBuf[j] < 0x00) {
                        carry = 1;
                        numBuf[j] += 0x100;
                    } else carry = 0;
                }
            }
    
            for (var k = 0; k < 8; k++)
                this.buffer[k] = numBuf[k]
    
            return this;
        },

        toString: function() {
            out = ""
            for (var i in this.buffer) {
                out = this.buffer[i].toString(16) + out;
            }
            return "0x" + out;
        }
    }
}

var RAM = {
    readU8: function(addr) {
        var buf = readNBytes(addr.buffer, 1);
        return buf[0];
    },     
    readU16: function(addr) {
        var buf = readNBytes(addr.buffer, 2);
        return buf[0] | buf[1] << 8;
    },
    readU32: function(addr) {
        var buf = readNBytes(addr.buffer, 4);
        return buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
    },
    readU64: function(addr) {
        var buf = readNBytes(addr.buffer, 8);
        return u64(buf);
    },
    writeU8: function(addr, value) {
        var buf = new Uint8Array(1);
        buf[0] = value & 0xFF;

        writeNBytes(addr.buffer, 1, buf);
    },     
    writeU16: function(addr, value) {
        var buf = new Uint8Array(2);
        buf[0] = value & 0xFF;
        buf[1] = (value >> 8) & 0xFF;
        
        writeNBytes(addr.buffer, 2, buf);
    },
    writeU32: function(addr, value) {
        var buf = new Uint8Array(4);
        buf[0] = value & 0xFF;
        buf[1] = (value >> 8) & 0xFF;
        buf[2] = (value >> 16) & 0xFF;
        buf[3] = (value >> 24) & 0xFF;
        
        writeNBytes(addr.buffer, 4, buf);
    },
    writeU64: function(addr, value) {        
        writeNBytes(addr.buffer, 8, value.buffer);
    }
};

print("Loaded!");

function testFunc() {
    print("Called!");
    print("MAIN: " + Addresses.MAIN);
    addr = u64(Addresses.MAIN);
	var_15 = RAM.readU64(addr.add(0x26583c0));
    print("addr: " + addr)
	var_15 = var_15.add(0x642d3c);
	RAM.writeU32(var_15, 0x6);	
}
