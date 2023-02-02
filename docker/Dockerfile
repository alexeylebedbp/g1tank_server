FROM alexeylebed87/ubuntu_with_boost

ARG DEBIAN_FRONTEND=noninteractive

RUN \
    export GIT_SSL_NO_VERIFY=1 \
    && git clone https://github.com/alexeylebedbp/g1tank_server.git \
    && cd g1tank_server \
    && mkdir third_party \
    && cd third_party \
    && git clone https://github.com/nlohmann/json.git \
    && cd ../ \
    && cmake -DCMAKE_C_COMPILER=/usr/bin/gcc-10 -DCMAKE_CXX_COMPILER=/usr/bin/g++-10  . . \
    && make


CMD g1tank_server/G1Tank


