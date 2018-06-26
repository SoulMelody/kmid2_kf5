#
# spec file for package kmid (Version 2.4.0)
#
# norootforbuild

Name:           kmid
Version:        2.4.0
Release:        1
License:        GPL v2 or later
Summary:        KDE MIDI/Karaoke Player
Group:          Productivity/Multimedia/Sound/Midi
URL:            http://kmid2.sourceforge.net
Source:         %name-%version.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
BuildRequires:  cmake
BuildRequires:  alsa-devel
BuildRequires:  libkde4-devel
BuildRequires:  libdrumstick-devel
%kde4_runtime_requires

%description
KMid is a MIDI/Karaoke player with KDE interface, based on the ALSA
sequencer.


Authors:
--------
    Antonio Larrosa Jimenez <larrosa@kde.org>
    Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>

%package -n libkmidbackend1
Summary:        KDE MIDI/Karaoke Player Backend
Group:          System/Libraries

%description -n libkmidbackend1
KMid is a MIDI/Karaoke player with KDE interface, based on the ALSA
sequencer.


Authors:
--------
    Antonio Larrosa Jimenez <larrosa@kde.org>
    Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>

%package -n libkmidbackend-devel
Summary:        Development package for the libkmidbackend library
Group:          Development/Libraries/C and C++
Requires:       libkmidbackend1 = %{version}
Requires:       glibc-devel libstdc++-devel libkde4-devel

%description -n libkmidbackend-devel
This package contains the files needed to compile KMid backends.


Authors:
--------
    Antonio Larrosa Jimenez <larrosa@kde.org>
    Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>

%lang_package

%prep
%setup -q

%build
%cmake_kde4
%make_jobs

%install
%makeinstall
%suse_update_desktop_file -G "MIDI/Karaoke Player" %name AudioVideo Music
rm -f %{buildroot}/usr/share/doc/kde/HTML/*/%{name}/common
%find_lang %name
%kde_post_install

%post -n libkmidbackend1 -p /sbin/ldconfig

%postun -n libkmidbackend1 -p /sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README TODO ChangeLog COPYING
%dir %{_datadir}/kde4/apps/%name
%dir %{_datadir}/kde4/apps/kmid_part
%{_bindir}/%name
%{_libdir}/kde4/*
%{_datadir}/applications/*/%name.desktop
%{_datadir}/kde4/apps/%name/*
%{_datadir}/kde4/apps/kmid_part/*
%{_datadir}/kde4/config.kcfg/*
%{_datadir}/kde4/services/*
%{_datadir}/kde4/servicetypes/*
%{_datadir}/icons/hicolor/*/*/*
%{_datadir}/dbus-1/interfaces/*

%files -n libkmidbackend1
%defattr(-,root,root)
%{_libdir}/libkmidbackend.so.*

%files -n libkmidbackend-devel
%defattr(-,root,root)
%dir %{_includedir}/kmid
%{_libdir}/libkmidbackend.so
%{_includedir}/kmid/*.h

%files lang -f %{name}.lang

%changelog
* Sun Aug 15 2010 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net> 2.4.0
- New version: 2.4.0
* Mon Apr 26 2010 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net> 2.3.0
- New version: 2.3.0
* Sun Mar 14 2010 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net> 2.2.2
- Renamed: kmid2->kmid
- New version: 2.2.2 
* Mon Feb 8 2010 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net> 0.2.1
- New version
* Wed Jan 27 2010 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net> 0.2.0
- New version
* Wed Dec 30 2009 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net> 0.1.1
- New version
* Fri Dec 18 2009 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net> 0.1.0-2
- openSUSE build service project restructuration
* Mon Nov 30 2009 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net> 0.1.0
- First release
