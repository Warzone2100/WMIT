FROM ubuntu:18.04

RUN cat /etc/lsb-release

RUN apt-get -u update \
 && DEBIAN_FRONTEND=noninteractive apt-get -y install build-essential automake ninja-build qt5-default git cmake sudo \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /code
CMD ["sh"]

