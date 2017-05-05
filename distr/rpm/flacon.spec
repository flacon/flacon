Name:           flacon
Version:        1.0.1
Release:        1%{?dist}
Summary:        Flacon extracts individual tracks from one big audio file containing the entire album of music and saves them as separate audio files.
Group:          Applications/Multimedia
Packager:       George Machitidze <giomac@gmail.com>
License:        LGPL-2.1
URL:            https://github.com/flacon/flacon
Source0:        https://github.com/flacon/flacon/archive/flacon-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  cmake qt-devel pkgconfig gcc-c++ libuchardet-devel
Requires:       shntool, flac, mac, uchardet
Suggests:       vorbis-tools, wavpack, lame, vorbisgain, mp3gain, ttaenc, faac

%description
Audio file splitter and converter
Flacon extracts individual tracks from one big audio file containing
the entire album of music and saves them as separate audio files.
To do this, it uses information from the appropriate CUE file.

%prep
%autosetup

%build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} ..
make %{?_smp_mflags}

%install
cd build
make install DESTDIR=$RPM_BUILD_ROOT 

%clean
rm -rf $RPM_BUILD_ROOT

%files
%_bindir/%{name}
%_datarootdir

%changelog
* Sun May 17 2015 George Machitidze <giomac@gmail.com> 1.0.1
- Initial release
