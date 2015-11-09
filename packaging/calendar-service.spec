Name:       calendar-service
Summary:    DB library for calendar
Version:    0.1.153
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    calendar-serviced.service
Source2:    calendar-serviced.socket
Source3:    ALARM.acalendar-service.service

%if "%{?tizen_profile_name}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

Requires(post): /sbin/ldconfig
Requires(post): /usr/bin/sqlite3, /bin/chown
Requires(post): contacts-service2
Requires(postun): /sbin/ldconfig

BuildRequires: cmake
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(pims-ipc)
BuildRequires: pkgconfig(accounts-svc)
BuildRequires: pkgconfig(contacts-service2)
BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(capi-base-common)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(libsmack)

%description
DB library for calendar

%package devel
Summary:    DB library for calendar
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(alarm-service)

%description devel
DB library for calendar (developement files)

%prep
%setup -q


%build
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"

MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DBIN_INSTALL_DIR:PATH=%{_bindir} \
		-DMAJORVER=${MAJORVER} \
		-DFULLVER=%{version}


make %{?jobs:-j%jobs}

%install
%make_install

mkdir -p %{buildroot}%{_unitdir_user}/default.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_unitdir_user}/calendar-serviced.service
ln -s ../calendar-serviced.service %{buildroot}%{_unitdir_user}/default.target.wants/calendar-serviced.service

mkdir -p %{buildroot}%{_unitdir_user}/sockets.target.wants
install -m 0644 %SOURCE2 %{buildroot}%{_unitdir_user}/calendar-serviced.socket
ln -s ../calendar-serviced.socket %{buildroot}%{_unitdir_user}/sockets.target.wants/calendar-serviced.socket

mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}

# alarm dbus service file
mkdir -p %{buildroot}/usr/share/dbus-1/system-services
cp -a %SOURCE3 %{buildroot}/usr/share/dbus-1/system-services

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest calendar-service.manifest
%defattr(-,root,root,-)
%{_bindir}/calendar-serviced*
%{_libdir}/libcalendar-service2.so.*
%{_unitdir_user}/default.target.wants/calendar-serviced.service
%{_unitdir_user}/calendar-serviced.service
%{_unitdir_user}/sockets.target.wants/calendar-serviced.socket
%{_unitdir_user}/calendar-serviced.socket
/usr/share/license/%{name}
/usr/share/dbus-1/system-services/ALARM.acalendar-service.service

%files devel
%defattr(-,root,root,-)
%{_includedir}/calendar-service2/*.h
%{_libdir}/*.so
%{_libdir}/pkgconfig/calendar-service2.pc
