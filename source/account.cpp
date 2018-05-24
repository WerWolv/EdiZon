#include "account.hpp"

#include <cstring>

extern "C" {
  #include "nanojpeg.h"
}

Account *Account::g_currAccount = nullptr;

Account::Account(u128 userID) : m_userID(userID) {
  accountInitialize();

  accountGetProfile(&m_profile, userID);
  accountProfileGet(&m_profile, &m_userData, &m_profileBase);
  accountProfileGetImageSize(&m_profile, &m_profileImageSize);

  m_userName = std::string(m_profileBase.username, 0x20);

  u8 *buffer = (u8*) malloc(m_profileImageSize);
  u8 *decodedBuffer = nullptr;
  size_t imageSize = 0;

  accountProfileLoadImage(&m_profile, buffer, m_profileImageSize, &imageSize);
  njInit();
  njDecode(buffer, imageSize);

  decodedBuffer = njGetImage();
  m_profileImage = (u8*) malloc(128*128*3);
  resizeImage(decodedBuffer, m_profileImage, 256, 256, 128, 128);

  njDone();

  accountProfileClose(&m_profile);
  accountExit();
}

Account::~Account() {

}

u128 Account::getUserID() {
  return m_userID;
}

std::string Account::getUserName() {
  return m_userName;
}

u8* Account::getProfileImage() {
  return m_profileImage;
}

inline unsigned char getpixel(u8* in, size_t src_width, size_t src_height, unsigned x, unsigned y, int channel) {
    if (x < src_width && y < src_height)
        return in[(x * 3 * src_width) + (3 * y) + channel];

    return 0;
}

void Account::resizeImage(u8* in, u8* out, size_t src_width, size_t src_height, size_t dest_width, size_t dest_height) {
    const float tx = float(src_width) / dest_width;
    const float ty = float(src_height) / dest_height;
    const int channels = 3;
    const std::size_t row_stride = dest_width * channels;

    unsigned char C[5] = { 0 };

    for (unsigned int i = 0; i < dest_height; ++i)
    {
        for (unsigned int j = 0; j < dest_width; ++j)
        {
            const int x = int(tx * j);
            const int y = int(ty * i);
            const float dx = tx * j - x;
            const float dy = ty * i - y;

            for (int k = 0; k < 3; ++k)
            {
                for (int jj = 0; jj < 4; ++jj)
                {
                    const int z = y - 1 + jj;
                    unsigned char a0 = getpixel(in, src_width, src_height, z, x, k);
                    unsigned char d0 = getpixel(in, src_width, src_height, z, x - 1, k) - a0;
                    unsigned char d2 = getpixel(in, src_width, src_height, z, x + 1, k) - a0;
                    unsigned char d3 = getpixel(in, src_width, src_height, z, x + 2, k) - a0;
                    unsigned char a1 = -1.0 / 3 * d0 + d2 - 1.0 / 6 * d3;
                    unsigned char a2 = 1.0 / 2 * d0 + 1.0 / 2 * d2;
                    unsigned char a3 = -1.0 / 6 * d0 - 1.0 / 2 * d2 + 1.0 / 6 * d3;
                    C[jj] = a0 + a1 * dx + a2 * dx * dx + a3 * dx * dx * dx;

                    d0 = C[0] - C[1];
                    d2 = C[2] - C[1];
                    d3 = C[3] - C[1];
                    a0 = C[1];
                    a1 = -1.0 / 3 * d0 + d2 -1.0 / 6 * d3;
                    a2 = 1.0 / 2 * d0 + 1.0 / 2 * d2;
                    a3 = -1.0 / 6 * d0 - 1.0 / 2 * d2 + 1.0 / 6 * d3;
                    out[i * row_stride + j * channels + k] = a0 + a1 * dy + a2 * dy * dy + a3 * dy * dy * dy;
                }
            }
        }
    }
}
