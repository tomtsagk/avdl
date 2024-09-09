FROM afloofdev/avdl_ubuntu
WORKDIR /idle
COPY . .
VOLUME /result
CMD avdl --android && cp avdl_build_android/* /result
