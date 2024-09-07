FROM afloofdev/avdl_ubuntu
WORKDIR /idle
COPY . .
VOLUME /result
CMD avdl && mkdir avdl_build/dependencies && cp -r /avdl/avdl_dependencies_linux/linux/lib/*.so* avdl_build/dependencies && cp -r avdl_build /result
