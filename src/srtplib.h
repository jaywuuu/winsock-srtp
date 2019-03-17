#pragma once

#include <windows.h>

#include "srtp.h"
#include "srtp_priv.h"

#define SRTP_MSG_HEADER_SIZE sizeof(srtp_hdr_t) + sizeof(size_t)

class SRTP
{
public:
    static const size_t max_msg_length = 16384; // 16kb

    struct srtp_msg
    {
        srtp_hdr_t header;
        size_t body_size;
        char body[max_msg_length];
    };

public:
    SRTP();
    ~SRTP();

    SRTP(const SRTP& srtp) = delete;
    SRTP& operator=(const SRTP& srtp) = delete;

    SRTP(SRTP&& srtp) = delete;
    SRTP& operator=(SRTP&& srtp) = delete;

    static const int max_key_length = 30;
    static uint8_t key[max_key_length];

    bool CreateSession();
    bool Protect(const void* input, size_t input_len, srtp_msg& output);
    bool Unprotect(srtp_msg& msg, char* unprot_msg, int& unprot_msg_len);

    void InitHeader(srtp_hdr_t& header) const;

private:
    srtp_t m_session;
    srtp_policy_t m_policy;
};