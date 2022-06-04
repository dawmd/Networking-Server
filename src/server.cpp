#include "messages/serializer.h"
#include "utilities/byte_inserter.h"
#include "utilities/smart_struct.h"
#include <deque>
#include <messages/server_messages.h>
#include <iostream>

using namespace SK;

#define GET_FIELD(field) get<#field>()

int main() {
    Position pos{};
    pos.GET_FIELD(x) = 2;
    pos.GET_FIELD(y) = 3;

    std::deque<std::byte> d{};
    ByteQueue bq{d};
    d.push_back(std::byte{0});
    d.push_back(std::byte{23});
    d.push_back(std::byte{0});
    d.push_back(std::byte{10});

    auto x = Serializer<Position>::deserialize(bq);
    std::cout << "x: " << x.GET_FIELD(x) << ", " << "y: " << x.GET_FIELD(y) << "\n";
}