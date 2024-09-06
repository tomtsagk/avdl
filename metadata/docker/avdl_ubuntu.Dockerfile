FROM afloofdev/avdl_ubuntu_dependencies
WORKDIR /avdl
COPY . .
ENV DEBIAN_FRONTEND=noninteractive 
ENV ANDROID_HOME=/avdl/android
RUN make -j6 prefix=/usr CC=gcc && make -j6 prefix=/usr CC=gcc install && avdl --version
