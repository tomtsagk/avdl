FROM ubuntu:20.04
WORKDIR /avdl
COPY . .
ENV DEBIAN_FRONTEND=noninteractive 
VOLUME /avdl/avdl_volume
RUN ./scripts/install_dependencies_ubuntu.sh
CMD ./scripts/dependencies_linux.sh && cp -r avdl_build_dependencies/avdl_dependencies_linux /avdl/avdl_volume
