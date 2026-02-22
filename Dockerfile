FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++-14 gcc-14 cmake make \
    clang-tidy clang-format cppcheck iwyu \
    git ca-certificates \
  && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100 \
  && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 \
  && rm -rf /var/lib/apt/lists/*
