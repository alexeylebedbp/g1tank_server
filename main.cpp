#include <iostream>
#include "pthread.h"
#include "Server.h"
#include "include_boost_asio.h"

int main() {
    auto server = make_shared<Server>();
    server->run();
    return 0;
}
