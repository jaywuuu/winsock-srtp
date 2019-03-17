#include "srtplib.h"

#include <stdexcept>

// Set key to predetermined value
uint8_t SRTP::key[max_key_length] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                   0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D };

SRTP::SRTP()
{
    if (srtp_init() != srtp_err_status_ok)
        throw std::runtime_error("Cannot initialize SRTP library.");

    // clear session
    memset(&m_session, 0x0, sizeof(srtp_t));

    // default policy values
    memset(&m_policy, 0x0, sizeof(srtp_policy_t));

    // ssrc
    srtp_ssrc_t ssrc = {};
    ssrc.type = ssrc_specific;
    ssrc.value = 0xdeadbeef;

    // set policy to describe a policy for an SRTP stream
    srtp_crypto_policy_set_rtp_default(&m_policy.rtp);
    srtp_crypto_policy_set_rtcp_default(&m_policy.rtcp);
    m_policy.ssrc = ssrc;
    m_policy.key = key;
    m_policy.next = NULL;
}

SRTP::~SRTP()
{
    srtp_shutdown();
}

bool SRTP::CreateSession()
{
    if (srtp_create(&m_session, &m_policy) != srtp_err_status_ok)
        return false;

    return true;
}

bool SRTP::Protect(const void* input, size_t input_len, srtp_msg& output)
{
    memcpy(&output.body, input, input_len);

    output.header.seq = ntohs(output.header.seq) + 1;
    output.header.seq = htons(output.header.seq);
    output.header.ts = ntohl(output.header.ts) + 1;
    output.header.ts = htonl(output.header.ts);

    int len = SRTP_MSG_HEADER_SIZE + (int)input_len;
    
    if (srtp_protect(m_session, &output, &len) != srtp_err_status_ok)
        return false;

    if (len == 0)
        return false;

    output.body_size = len;

    return true;
}

bool SRTP::Unprotect(srtp_msg& msg, char* unprot_msg, int& unprot_msg_len)
{
    // header verification
    if (msg.header.version != 2)
        return false;

    int len = msg.body_size;

    if (srtp_unprotect(m_session, &msg.header, &len) != srtp_err_status_ok)
        return false;

    unprot_msg_len = len;

    memcpy(unprot_msg, msg.body, len);

    return true;
}

void SRTP::InitHeader(srtp_hdr_t& header) const
{
    header.ssrc = htonl(m_policy.ssrc.value);
    header.ts = 0;
    header.seq = (uint16_t)rand();
    header.m = 0;
    header.pt = 0x1;
    header.version = 2;
    header.p = 0;
    header.x = 0;
    header.cc = 0;
}