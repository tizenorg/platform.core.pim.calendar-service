Name:       calendar-service
Summary:    DB library for calendar
Version:    0.1.15
Release:    1
Group:      Social & Content/Calendar
License:    Apache 2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    calendar.service
Source1001: 	calendar-service.manifest
Requires(post): /sbin/ldconfig
Requires(post): /usr/bin/sqlite3, /bin/chown
Requires(postun): /sbin/ldconfig

BuildRequires: cmake
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(capi-base-common)
BuildRequires: pkgconfig(contacts-service2)
BuildRequires: pkgconfig(pims-ipc)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(libtzplatform-config)

%description
DB library for calendar

%package devel
Summary:    DB library for calendar
Group:      Social & Content/Calendar
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(alarm-service)

%description devel
DB library for calendar (developement files)

%prep
%setup -q
cp %{SOURCE1001} .


%build
%cmake .


make %{?jobs:-j%jobs}

%install
%make_install

mkdir -p %{buildroot}/etc/rc.d/rc3.d/
mkdir -p %{buildroot}/etc/rc.d/rc5.d/
ln -s ../init.d/calendar-serviced.sh %{buildroot}/etc/rc.d/rc3.d/S85calendar-serviced
ln -s ../init.d/calendar-serviced.sh %{buildroot}/etc/rc.d/rc5.d/S85calendar-serviced

mkdir -p %{buildroot}%{_unitdir_user}/tizen-middleware.target.wants
install %{SOURCE1} %{buildroot}%{_unitdir_user}/
ln -s ../calendar.service %{buildroot}%{_unitdir_user}/tizen-middleware.target.wants/

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
#%{_libdir}/libcalendar-service-native.so.*
%{_bindir}/calendar-serviced*
%{_libdir}/libcalendar-service2.so.*
%attr(0755,root,root) /etc/rc.d/init.d/calendar-serviced.sh
/etc/rc.d/rc3.d/S85calendar-serviced
/etc/rc.d/rc5.d/S85calendar-serviced
/usr/share/calendar-svc/dft-calendar
%{_unitdir_user}/calendar.service
%{_unitdir_user}/tizen-middleware.target.wants/calendar.service

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/calendar-service/*.h
%{_includedir}/calendar-service2/*.h
%{_libdir}/*.so
%{_libdir}/pkgconfig/calendar.pc
#%{_libdir}/pkgconfig/calendar-service-native.pc
%{_libdir}/pkgconfig/calendar-service2.pc
