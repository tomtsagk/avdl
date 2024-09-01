
set -e

apt-get install openjdk-17-jdk -y

mkdir android
mkdir android/cmdline-tools
mkdir android/cmdline-tools/latest

wget https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip
unzip commandlinetools-linux-11076708_latest.zip
mv cmdline-tools/* android/cmdline-tools/latest/
yes | ./android/cmdline-tools/latest/bin/sdkmanager --licenses
./android/cmdline-tools/latest/bin/sdkmanager --install "ndk;25.1.8937393"
./android/cmdline-tools/latest/bin/sdkmanager --install "platform-tools"
./android/cmdline-tools/latest/bin/sdkmanager --install "build-tools;30.0.3"
./android/cmdline-tools/latest/bin/sdkmanager --install "platforms;android-33"

git clone https://github.com/libsdl-org/freetype.git /avdl/freetype_quest2
