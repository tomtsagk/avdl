Name:           avdl
Version:        0.9.9
Release:        1%{?dist}
Summary:        Abstract video-game development language compiler

License:        GPLv3
URL:            https://afloofdev.com/avdl
Source0:        https://github.com/tomtsagk/%{name}/archive/refs/tags/v%{version}.tar.gz

BuildRequires:  make, gcc, glew-devel, SDL2-devel, SDL2_mixer-devel
Requires:       glew-devel, SDL2-devel, SDL2_mixer-devel, libpng-devel, libvorbis-devel, libogg-devel

%description
A compiler for the high level programming language with the same name.
It is used to compile scripts written in this language to full games.

%global debug_package %{nil}

%prep
%autosetup

%build
make %{?_smp_mflags} prefix=/usr

%install
rm -rf $RPM_BUILD_ROOT
make %{?_smp_mflags} prefix=/usr DESTDIR=%{buildroot} install

%files
/usr/bin/avdl
/usr/include/dd_*
/usr/include/avdl_*
/usr/share/avdl/android/*
/usr/share/avdl/quest2/*
/usr/share/avdl/cengine/*
/usr/share/avdl/templates/*
/usr/share/vim/vimfiles/ftdetect/avdl.vim
/usr/share/vim/vimfiles/syntax/avdl.vim
%license LICENSE
%doc /usr/share/man/man1/avdl.1.gz

%changelog
* Thu May 25 2023 Tom Tsagkatos <tomtsagkatos@afloof.dev>
- Fix performance issue on Quest 2

* Wed May 24 2023 Tom Tsagkatos <tomtsagkatos@afloof.dev>
- Add support for VR with Quest 2
- Can now open URL links from game
- Add Google Play Achievements

* Tue Feb 14 2023 Tom Tsagk <tomtsagk@afloofdev.com>
- Improve online functionality
- Automate builds
- Improve android builds

* Wed Aug 31 2022 Tom Tsagk <tomtsagk@afloofdev.com>
- Add achievements
- Re-organise some files to make compilation simpler

* Wed Jun 15 2022 Tom Tsagk <tomtsagk@darkdimension.org>
- Fix no audio bug for installed projects
- Fix no sfx bug for installed projects

* Tue Jun 14 2022 Tom Tsagk <tomtsagk@darkdimension.org>
- Switch text renderer to use bitmap fonts
- Separate music and sound audio
- Separate textures and meshes, to make them easier to manage.

* Sat Feb 18 2022 Tom Tsagk <tomtsagk@darkdimension.org>
- Fix bug that caused shaders to not render meshes on some hardware
- Add save/load functionality for android too

* Thu Feb 10 2022 Tom Tsagk <tomtsagk@darkdimension.org>
- Added `cmake` for better compilation options
- Bug fixes in regards to compilation
- Fix bug that added asset path twice

* Fri Feb 07 2022 Tom Tsagk <tomtsagk@darkdimension.org>
- Update to version `0.2.2`
- Improve automated compilation of games
- Fix localisation issue with compiled games

* Thu Jan 06 2022 Tom Tsagk <tomtsagk@darkdimension.org>
- Update to version `0.2.1`
- Created unit test to make the project more stable.
- The internal game engine is now compiled for each project to make cross compilation easier.

* Sun Aug 29 2021 Tom Tsagk <tomtsagk@darkdimension.org>
- Update to version `0.1.2`
- Removed the `freeglut` dependency, as it's no longer used.

* Sat Aug 21 2021 Tom Tsagk <tomtsagk@darkdimension.org>
- Update to version `0.0.7`

* Sat Aug 21 2021 Tom Tsagk <tomtsagk@darkdimension.org>
- Update to github source

* Sat Aug 07 2021 Tom Tsagk <tomtsagk@darkdimension.org>
- Initial Release
