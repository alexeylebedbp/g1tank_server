FROM ubuntu:20.04
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update

RUN \
  apt-get install -y --no-install-recommends \
    gcc-10 \
    g++-10 \
    libssl-dev \
    openssl \
    libtool \
    autoconf \
    automake \
    build-essential \
    freeglut3-dev \
    python3-dev \
    autotools-dev \
    cmake \
    libicu-dev

RUN apt-get install -y --no-install-recommends  git

RUN \
    export GIT_SSL_NO_VERIFY=1 \
    && git clone --recursive https://github.com/boostorg/boost.git \
    && cd boost/libs \
    && git clone https://github.com/djarek/certify.git \
    && cd ../ \
    && ./bootstrap.sh \
    && ./b2 install

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


