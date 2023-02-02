#Docker Hub Image name: alexeylebed87/ubuntu_with_boost

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

CMD echo "Ubuntu 20.04 with gcc-10 and boost docker image"

