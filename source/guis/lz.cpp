// #include <switch.h>
// #include "pch.h"
#include <inttypes.h>
//#include <string>
typedef uint64_t u64;
/*************************************************************************
* LZ_Compress() - Compress a block of data using an LZ8 coder.
*  in     - Input (uncompressed) buffer.
*  out    - Output (compressed) buffer. This buffer must be 1/8 larger
*           than the input buffer to cater for the worst case. 
*  insize - Number of input bytes.
* The function returns the size of the compressed data.
*************************************************************************/
int LZ_Compress(const unsigned char *in, unsigned char *out, unsigned int insize) {
    unsigned int inpos, outpos;
    struct marker_t {
        unsigned char back : 4;
        unsigned char front : 4;
    } marker = {0};
#define MAXRANGE 16 
    if (insize < 1) {
        return 0;
    }
    inpos = 0;
    outpos = 0;
    do {
        marker.front = 8;
        for (int front = 0; front <= 8; front++) {
            for (unsigned int back = 1; back <= MAXRANGE; back++) {
                if (inpos < back * 8) {
                    break;
                }
                if ((*(u64 *)(&in[inpos - back * 8]) & (0xFFFFFFFFFFFFFFFF << 8 * front)) == (*(u64 *)(&in[inpos]) & (0xFFFFFFFFFFFFFFFF << 8 * front))) {
                    marker.front = front;
                    marker.back = back - 1;
                    front = 8;
                    break;
                }
            }
        }
        out[outpos] = *(unsigned char *)(&marker);
        outpos += sizeof(marker_t);
        *(unsigned long long *)(&out[outpos]) = *(unsigned long long *)(&in[inpos]);
        outpos += marker.front;
        inpos += 8;
    } while (inpos < MAXRANGE*8);
    do {
        marker.front = 8;
        for (int front = 0; front <= 8; front++) {
            for (unsigned int back = 1; back <= MAXRANGE; back++) {
                //if (inpos < back * 8) {
                //    break;
                //}
                if ((*(u64*)(&in[inpos - back * 8]) & (0xFFFFFFFFFFFFFFFF << 8 * front)) == (*(u64*)(&in[inpos]) & (0xFFFFFFFFFFFFFFFF << 8 * front))) {
                    marker.front = front;
                    marker.back = back - 1;
                    front = 8;
                    break;
                }
            }
        }
        out[outpos] = *(unsigned char*)(&marker);
        outpos += sizeof(marker_t);
        *(unsigned long long*)(&out[outpos]) = *(unsigned long long*)(&in[inpos]);
        outpos += marker.front;
        inpos += 8;
    } while (inpos < insize);
    return outpos;
}
/*************************************************************************
* LZ_Uncompress() - Uncompress a block of data using an LZ8 decoder.
*  in      - Input (compressed) buffer.
*  out     - Output (uncompressed) buffer. This buffer must be large
*            enough to hold the uncompressed data.
*  insize  - Number of input bytes.
**************************************************************************/
int LZ_Uncompress(const unsigned char *in, unsigned char *out, unsigned int insize) {
    unsigned int inpos, outpos;
    struct marker_t {
        unsigned char back : 4;
        unsigned char front : 4;
    } marker = {0};
    if (insize < 1) {
        return 0;
    }
     //marker = *(marker_t *)(&in[0]);
    inpos = 0;
    outpos = 0;
    do {
        marker = *(marker_t *)(&in[inpos]);
        inpos++;
        if (marker.front == 8) {
            *(u64 *)(&out[outpos]) = *(u64 *)(&in[inpos]);
        } else {
            *(u64 *)(&out[outpos]) = (*(u64 *)(&in[inpos]) & ~(0xFFFFFFFFFFFFFFFF << (8 * marker.front))) | (*(u64 *)(&out[outpos - (marker.back + 1) * 8]) & (0xFFFFFFFFFFFFFFFF << (8 * marker.front)));
        }
        inpos += marker.front;
        outpos += 8;
    } while (inpos < insize);
    return outpos;
}
