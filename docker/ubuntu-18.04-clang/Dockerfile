FROM ubuntu:18.04

RUN cat /etc/lsb-release

RUN apt-get -u update \
 && DEBIAN_FRONTEND=noninteractive apt-get -y install clang ninja-build qt5-default git cmake sudo \
 && rm -rf /var/lib/apt/lists/*

ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++

WORKDIR /code
CMD ["sh"]

