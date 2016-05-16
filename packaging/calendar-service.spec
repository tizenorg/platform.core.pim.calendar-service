Name:       calendar-service
Summary:    DB library for calendar
Version:    0.1.162
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    %{name}d.service
Source2:    org.tizen.CalendarService.dbus.service
Source1001: %{name}.manifest
Source1002: %{name}.conf.in
Source2001: %{name}-alarm.service
Source2002: ALARM.a%{name}.service
%if "%{?profile}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif
BuildRequires: cmake
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(accounts-svc)
BuildRequires: pkgconfig(contacts-service2)
BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(capi-base-common)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(libsmack)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%define _dbus_interface org.tizen.CalendarService.dbus

%description
Calendar Service for using Calendar DB

%package devel
Summary:    DB library for calendar
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(alarm-service)

%description devel
Calendar Service for using Calendar DB(development Kit)


%prep
%setup -q
chmod g-w %_sourcedir/*
cp %{SOURCE1001} .


%build
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"

MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DMAJORVER=${MAJORVER} -DFULLVER=%{version} -DBIN_INSTALL_DIR:PATH=%{_bindir} \
		-DDBUS_INTERFACE=%{_dbus_interface}


make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_unitdir_user}/default.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_unitdir_user}

mkdir -p %{buildroot}%{_datadir}/dbus-1/services
install -m 0644 %SOURCE2 %{buildroot}%{_datadir}/dbus-1/services

mkdir -p %{buildroot}/%{_sysconfdir}/dbus-1/session.d
sed -i 's/@DBUS_INTERFACE@/%{_dbus_interface}/g' %{SOURCE1002}
install -m 0644 %{SOURCE1002} %{buildroot}%{_sysconfdir}/dbus-1/session.d/%{name}.conf

# alarm dbus service file
install -m 0644 %SOURCE2001 %{buildroot}%{_unitdir_user}
mkdir -p %{buildroot}%{_datadir}/dbus-1/system-services
install -m 0644 %SOURCE2002 %{buildroot}%{_datadir}/dbus-1/system-services


%post -p /sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_bindir}/calendar-serviced*
%{_libdir}/lib%{name}2.so.*
%{_unitdir_user}/%{name}d.service
%{_datadir}/dbus-1/services/%{_dbus_interface}.service
%config %{_sysconfdir}/dbus-1/session.d/%{name}.conf
%{_unitdir_user}/%{name}-alarm.service
%{_datadir}/dbus-1/system-services/ALARM.acalendar-service.service
%license LICENSE.APLv2


%files devel
%defattr(-,root,root,-)
%{_includedir}/calendar-service2/*.h
%{_libdir}/*.so
%{_libdir}/pkgconfig/calendar-service2.pc
