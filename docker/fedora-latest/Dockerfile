FROM fedora:latest

RUN cat /etc/fedora-release

RUN dnf -y update && dnf clean all
RUN dnf -y install git gcc gcc-c++ cmake ninja-build make \
 && dnf -y install qt5-devel mesa-libGLU-devel && dnf clean all

WORKDIR /code
CMD ["/bin/sh"]

