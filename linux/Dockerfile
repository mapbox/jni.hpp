FROM ubuntu:xenial-20171201

ENV DEBIAN_FRONTEND="noninteractive"

# --------------------------------------------------------------------------------------------------
# Install common packages

COPY *.list /etc/apt/sources.list.d/
COPY *.gpg /tmp/
RUN set -eu \
 && (find /tmp/*.gpg | xargs -n1 apt-key add) \
 && rm /tmp/*.gpg

RUN set -eu \
 && apt-get update \
 && apt-get -y install git make locales openjdk-8-jdk-headless \
                       clang-3.8 clang-3.9 clang-4.0 clang-5.0 \
                       g++-4.9 g++-5 g++-6 g++-7 \
 && rm -rf /var/lib/apt/lists/*

# --------------------------------------------------------------------------------------------------
# Setup environment

RUN locale-gen en_US.UTF-8

ENV LC_ALL="en_US.UTF-8" \
    CC="gcc-5" \
    CXX="g++-5"

WORKDIR /src
