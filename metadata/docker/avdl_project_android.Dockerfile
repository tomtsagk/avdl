FROM afloofdev/avdl_ubuntu
WORKDIR /idle
COPY . .
VOLUME /result
CMD avdl --android && cd avdl_build_android && chmod +x gradlew && ./gradlew build && cp app/build/outputs/apk/release/app-universal-release-unsigned.apk /result/idle-release-unsigned.apk && cp app/build/outputs/apk/debug/app-universal-debug.apk /result/idle-debug.apk && cp app/build/outputs/native-debug-symbols/release/native-debug-symbols.zip /result && cp app/build/outputs/mapping/release/mapping.txt /result
