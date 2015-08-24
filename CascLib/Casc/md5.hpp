/*
* Copyright 2015 Gunnar Lilleaasen
*
* This file is part of CascLib.
*
* CascLib is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* CascLib is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CascLib.  If not, see <http://www.gnu.org/licenses/>.
*
* Based on:
*
* MD5
* converted to C++ class by Frank Thilo (thilo@unix-ag.org)
* for bzflag (http://www.bzflag.org)
*
* based on:
*
* md5.h and md5.c
* reference implementation of RFC 1321
* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
* rights reserved.
*
* License to copy and use this software is granted provided that it
* is identified as the "RSA Data Security, Inc. MD5 Message-Digest
* Algorithm" in all material mentioning or referencing this software
* or this function.
*
* License is also granted to make and use derivative works provided
* that such works are identified as "derived from the RSA Data
* Security, Inc. MD5 Message-Digest Algorithm" in all material
* mentioning or referencing the derived work.
*
* RSA Data Security, Inc. makes no representations concerning either
* the merchantability of this software or the suitability of this
* software for any particular purpose. It is provided "as is"
* without express or implied warranty of any kind.
*
* These notices must be retained in any copies of any part of this
* documentation and/or software.
*/

#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>
#include <vector>

// Constants for MD5Transform routine.
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

class MD5
{
public:
    typedef unsigned int size_type; // must be 32bit

    MD5()
    {
        init();
    }
    MD5(const std::string& text)
    {
        init();
        update(text.c_str(), text.length());
        finalize();
    }
    MD5(const std::vector<char>& input)
    {
        init();
        update(input.data(), input.size());
        finalize();
    }
    MD5(std::ifstream &stream, size_type length)
    {
        init();

        auto pos = stream.tellg();

        char buf[4096];
        for (auto i = 0U; i < length; i += 4096U)
        {
            auto count = std::min(length - i, 4096U);

            stream.read(buf, count);
            update(buf, count);
        }

        stream.seekg(pos);

        finalize();
    }
    void update(const unsigned char *input, size_type length)
    {
        // compute number of bytes mod 64
        size_type index = count[0] / 8 % blocksize;

        // Update number of bits
        if ((count[0] += (length << 3)) < (length << 3))
            count[1]++;
        count[1] += (length >> 29);

        // number of bytes we need to fill in buffer
        size_type firstpart = 64 - index;

        size_type i;

        // transform as many times as possible.
        if (length >= firstpart)
        {
            // fill buffer first, transform
            memcpy(&buffer[index], input, firstpart);
            transform(buffer);

            // transform chunks of blocksize (64 bytes)
            for (i = firstpart; i + blocksize <= length; i += blocksize)
                transform(&input[i]);

            index = 0;
        }
        else
            i = 0;

        // buffer remaining input
        memcpy(&buffer[index], &input[i], length - i);
    }
    void update(const char *input, size_type length)
    {
        update((const unsigned char*)input, length);
    }
    void update(const std::vector<char> &input)
    {
        update((const unsigned char*)&input[0], input.size());
    }
    MD5& finalize()
    {
        static unsigned char padding[64] = {
            0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };

        if (!finalized) {
            // Save number of bits
            unsigned char bits[8];
            encode(bits, count, 8);

            // pad out to 56 mod 64.
            size_type index = count[0] / 8 % 64;
            size_type padLen = (index < 56) ? (56 - index) : (120 - index);
            update(padding, padLen);

            // Append length (before padding)
            update(bits, 8);

            // Store state in digest
            encode(digest, state, 16);

            // Zeroize sensitive information.
            memset(buffer, 0, sizeof buffer);
            memset(count, 0, sizeof count);

            finalized = true;
        }

        return *this;
    }
    std::string hexdigest() const
    {
        if (!finalized)
            return "";

        std::stringstream ss;

        ss << std::hex
           << std::setfill('0');

        for (unsigned int c : digest)
        {
            ss << std::setw(2)
                << c;
        }

        return ss.str();
    }
    friend std::ostream& operator<<(std::ostream& out, MD5 md5)
    {
        return out << md5.hexdigest();
    }

private:
    void init()
    {
        finalized = false;

        count[0] = 0;
        count[1] = 0;

        // load magic initialization constants.
        state[0] = 0x67452301;
        state[1] = 0xefcdab89;
        state[2] = 0x98badcfe;
        state[3] = 0x10325476;
    }
    typedef unsigned char uint1; //  8bit
    typedef unsigned int uint4;  // 32bit
    enum { blocksize = 64 }; // VC6 won't eat a const static int here

    void transform(const uint1 block[blocksize])
    {
        uint4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];
        decode(x, block, blocksize);

        /* Round 1 */
        FF(a, b, c, d, x[0], S11, 0xd76aa478); /* 1 */
        FF(d, a, b, c, x[1], S12, 0xe8c7b756); /* 2 */
        FF(c, d, a, b, x[2], S13, 0x242070db); /* 3 */
        FF(b, c, d, a, x[3], S14, 0xc1bdceee); /* 4 */
        FF(a, b, c, d, x[4], S11, 0xf57c0faf); /* 5 */
        FF(d, a, b, c, x[5], S12, 0x4787c62a); /* 6 */
        FF(c, d, a, b, x[6], S13, 0xa8304613); /* 7 */
        FF(b, c, d, a, x[7], S14, 0xfd469501); /* 8 */
        FF(a, b, c, d, x[8], S11, 0x698098d8); /* 9 */
        FF(d, a, b, c, x[9], S12, 0x8b44f7af); /* 10 */
        FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
        FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
        FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
        FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
        FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
        FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

                                                /* Round 2 */
        GG(a, b, c, d, x[1], S21, 0xf61e2562); /* 17 */
        GG(d, a, b, c, x[6], S22, 0xc040b340); /* 18 */
        GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
        GG(b, c, d, a, x[0], S24, 0xe9b6c7aa); /* 20 */
        GG(a, b, c, d, x[5], S21, 0xd62f105d); /* 21 */
        GG(d, a, b, c, x[10], S22, 0x2441453); /* 22 */
        GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
        GG(b, c, d, a, x[4], S24, 0xe7d3fbc8); /* 24 */
        GG(a, b, c, d, x[9], S21, 0x21e1cde6); /* 25 */
        GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
        GG(c, d, a, b, x[3], S23, 0xf4d50d87); /* 27 */
        GG(b, c, d, a, x[8], S24, 0x455a14ed); /* 28 */
        GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
        GG(d, a, b, c, x[2], S22, 0xfcefa3f8); /* 30 */
        GG(c, d, a, b, x[7], S23, 0x676f02d9); /* 31 */
        GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

                                                /* Round 3 */
        HH(a, b, c, d, x[5], S31, 0xfffa3942); /* 33 */
        HH(d, a, b, c, x[8], S32, 0x8771f681); /* 34 */
        HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
        HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
        HH(a, b, c, d, x[1], S31, 0xa4beea44); /* 37 */
        HH(d, a, b, c, x[4], S32, 0x4bdecfa9); /* 38 */
        HH(c, d, a, b, x[7], S33, 0xf6bb4b60); /* 39 */
        HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
        HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
        HH(d, a, b, c, x[0], S32, 0xeaa127fa); /* 42 */
        HH(c, d, a, b, x[3], S33, 0xd4ef3085); /* 43 */
        HH(b, c, d, a, x[6], S34, 0x4881d05); /* 44 */
        HH(a, b, c, d, x[9], S31, 0xd9d4d039); /* 45 */
        HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
        HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
        HH(b, c, d, a, x[2], S34, 0xc4ac5665); /* 48 */

                                               /* Round 4 */
        II(a, b, c, d, x[0], S41, 0xf4292244); /* 49 */
        II(d, a, b, c, x[7], S42, 0x432aff97); /* 50 */
        II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
        II(b, c, d, a, x[5], S44, 0xfc93a039); /* 52 */
        II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
        II(d, a, b, c, x[3], S42, 0x8f0ccc92); /* 54 */
        II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
        II(b, c, d, a, x[1], S44, 0x85845dd1); /* 56 */
        II(a, b, c, d, x[8], S41, 0x6fa87e4f); /* 57 */
        II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
        II(c, d, a, b, x[6], S43, 0xa3014314); /* 59 */
        II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
        II(a, b, c, d, x[4], S41, 0xf7537e82); /* 61 */
        II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
        II(c, d, a, b, x[2], S43, 0x2ad7d2bb); /* 63 */
        II(b, c, d, a, x[9], S44, 0xeb86d391); /* 64 */

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;

        // Zeroize sensitive information.
        memset(x, 0, sizeof x);
    }
    static void decode(uint4 output[], const uint1 input[], size_type len)
    {
        for (unsigned int i = 0, j = 0; j < len; i++, j += 4)
            output[i] = ((uint4)input[j]) | (((uint4)input[j + 1]) << 8) |
                (((uint4)input[j + 2]) << 16) | (((uint4)input[j + 3]) << 24);
    }
    static void encode(uint1 output[], const uint4 input[], size_type len)
    {
        for (size_type i = 0, j = 0; j < len; i++, j += 4) {
            output[j] = input[i] & 0xff;
            output[j + 1] = (input[i] >> 8) & 0xff;
            output[j + 2] = (input[i] >> 16) & 0xff;
            output[j + 3] = (input[i] >> 24) & 0xff;
        }
    }

    bool finalized;
    uint1 buffer[blocksize]; // bytes that didn't fit in last 64 byte chunk
    uint4 count[2];   // 64bit counter for number of bits (lo, hi)
    uint4 state[4];   // digest so far
    uint1 digest[16]; // the result

                      // low level logic operations
    static inline uint4 F(uint4 x, uint4 y, uint4 z)
    {
        return (x&y) | (~x&z);
    }
    static inline uint4 G(uint4 x, uint4 y, uint4 z)
    {
        return (x&z) | (y&~z);
    }
    static inline uint4 H(uint4 x, uint4 y, uint4 z)
    {
        return x^y^z;
    }
    static inline uint4 I(uint4 x, uint4 y, uint4 z)
    {
        return y ^ (x | ~z);
    }
    static inline uint4 rotate_left(uint4 x, int n)
    {
        return (x << n) | (x >> (32 - n));
    }
    static inline void FF(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
    {
        a = rotate_left(a + F(b, c, d) + x + ac, s) + b;
    }
    static inline void GG(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
    {
        a = rotate_left(a + G(b, c, d) + x + ac, s) + b;
    }
    static inline void HH(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
    {
        a = rotate_left(a + H(b, c, d) + x + ac, s) + b;
    }
    static inline void II(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
    {
        a = rotate_left(a + I(b, c, d) + x + ac, s) + b;
    }
};

inline std::string md5(const std::string str)
{
    MD5 md5 = MD5(str);

    return md5.hexdigest();
}

inline std::string md5(const std::vector<char> &input)
{
    MD5 md5 = MD5(input);

    return md5.hexdigest();
}