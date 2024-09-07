FROM ubuntu:20.04
WORKDIR /avdl
COPY . .
ENV DEBIAN_FRONTEND=noninteractive 
ENV ANDROID_HOME=/avdl/android
RUN ./scripts/install_dependencies_ubuntu.sh && ./scripts/dependencies_linux.sh avdl_dependencies_linux && ./scripts/dependencies_android.sh
