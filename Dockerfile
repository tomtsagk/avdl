FROM ubuntu:20.04
WORKDIR /avdl
COPY . .
ENV DEBIAN_FRONTEND=noninteractive 
ENV ANDROID_HOME=/avdl/android
RUN ./scripts/install_dependencies_ubuntu.sh && ./scripts/dependencies_linux.sh && ./scripts/dependencies_android.sh && make -j6 prefix=/usr CC=gcc && make -j6 prefix=/usr CC=gcc install && avdl --version
