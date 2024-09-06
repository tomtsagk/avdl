FROM afloofdev/avdl_ubuntu
WORKDIR /idle
COPY . .
VOLUME /result
CMD avdl --quest2 && mkdir avdl_oculus_build && mv oculus_mobile_sdk.zip avdl_oculus_build && cd avdl_oculus_build && unzip oculus_mobile_sdk.zip && cp ../avdl_build_quest2/ -r avdl_vr_bk/AvdlProject && cp -r /avdl/freetype_quest2 avdl_vr_bk/AvdlProject/freetype && cd avdl_vr_bk/AvdlProject/freetype && git checkout fd08bbe13c52e305f037448e805e7e3c6ea1b1e4 && cd ../Projects/Android && chmod +x ../../../gradlew && ../../../gradlew assembleDebug && ../../../gradlew assembleRelease && cp build/outputs/apk/debug/Android-debug.apk /result/idle_quest2_debug.apk && cp build/outputs/apk/release/Android-release.apk /result/idle_quest2_unsigned.apk
