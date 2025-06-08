#include <cstdint>

struct RequestPayload
{
    char callType;
    char resendSeq;
};

#pragma pack(1)
struct ResponsePayload
{
    char symbol[5]; // size is kept 5 inorder to append a '\0' character at the end of the symbol
    char side;
    int32_t quantity;
    int32_t price;
    int32_t sequence;

    ResponsePayload()
    {
        memset(symbol, '\0', 5);
    }
};
#pragma pop()