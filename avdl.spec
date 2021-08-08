Name:           avdl
Version:        0.0.5
Release:        1%{?dist}
Summary:        Abstract video-game development language compiler

License:        GPLv3
URL:            https://darkdimension.org/avdl.html
Source0:        https://notabug.org/tomtsagk/%{name}/archive/v%{version}.tar.gz

BuildRequires:  make, gcc, glew-devel, freeglut-devel, SDL2-devel, SDL2_mixer-devel
Requires:       glew-devel, freeglut-devel, SDL2-devel, SDL2_mixer-devel

%description
A compiler for the high level programming language with the same name.
It is used to compile scripts written in this language to full games.

%global debug_package %{nil}

%prep
%autosetup -n %{name}


%build
make %{?_smp_mflags} prefix=/usr


%install
rm -rf $RPM_BUILD_ROOT
make %{?_smp_mflags} prefix=/usr DESTDIR=%{buildroot} install


%files
/usr/bin/avdl
/usr/include/dd_*
/usr/include/avdl_*
/usr/lib/libavdl-cengine.a
/usr/share/avdl/android/*
%license LICENSE
%doc /usr/share/man/man1/avdl.1.gz



%changelog
* Sat Aug 07 2021 tom
- Initial Release

