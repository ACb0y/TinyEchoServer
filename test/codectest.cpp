#include "codec.hpp"
#include "packet.hpp"
#include "unittestcore.hpp"

TEST_CASE(Codec_Init) {
  TinyEcho::Codec codec;
  ASSERT_EQ(codec.Len(), 4);
}

TEST_CASE(Codec_EnCodeAndDeCode) {
  TinyEcho::Codec codec;
  TinyEcho::Packet pkt;
  std::string message = "hello world";
  codec.EnCode(message, pkt);
  ASSERT_EQ(pkt.UseLen(), message.length() + 4);
  for (size_t i = 0; i < pkt.UseLen(); i++) {
    uint8_t* data = pkt.Data();
    *codec.Data() = *(data + i);
    codec.DeCode(1);
  }
  std::string* temp = codec.GetMessage();
  ASSERT_TRUE(temp != nullptr);
  ASSERT_EQ((*temp), message);
}

/*
 *   Codec() { pkt_.Alloc(TINY_ECHO_HEAD_LEN); }
  ~Codec() {
    if (msg_) delete msg_;
  }
  uint8_t *Data() { return pkt_.Data4Write(); }
  size_t Len() { return pkt_.CanWriteLen(); }
  void EnCode(const std::string &msg, Packet &pkt) {
    pkt.Alloc(msg.length() + TINY_ECHO_HEAD_LEN);
    *(uint32_t *)pkt.Data() = htonl(msg.length());  // 协议体长度转换成网络字节序
    memmove(pkt.Data() + TINY_ECHO_HEAD_LEN, msg.data(), msg.length());
  }
  void DeCode(size_t len) {
    uint8_t *data = (uint8_t *)pkt_.Data4Parse();
    uint32_t need_parse_len = pkt_.NeedParseLen();  // 还有多少字节需要解析
    while (need_parse_len > 0) {  // 只要还有未解析的网络字节流，就持续解析
      bool decode_break = false;
      if (HEAD == decode_status_) {  // 解析协议头
        decodeHead(&data, need_parse_len, decode_break);
        if (decode_break) break;
      }
      if (BODY == decode_status_) {  // 解析完协议头，解析协议体
        decodeBody(&data, need_parse_len, decode_break);
        if (decode_break) break;
      }
    }
  }
  std::string *GetMessage() {
    if (nullptr == msg_) return nullptr;
    if (decode_status_ != FINISH) return nullptr;
    std::string *result = msg_;
    msg_ = nullptr;
    return result;
  }
 */