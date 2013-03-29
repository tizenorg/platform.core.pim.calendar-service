Name:       calendar-service
Summary:    DB library for calendar
Version:    0.1.14
Release:    86
Group:      System/Libraries
License:    Apache 2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    calendar.service
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
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}


make %{?jobs:-j%jobs}

%install
%make_install

mkdir -p %{buildroot}/etc/rc.d/rc3.d/
mkdir -p %{buildroot}/etc/rc.d/rc5.d/
ln -s ../init.d/calendar-serviced.sh %{buildroot}/etc/rc.d/rc3.d/S85calendar-serviced
ln -s ../init.d/calendar-serviced.sh %{buildroot}/etc/rc.d/rc5.d/S85calendar-serviced

mkdir -p %{buildroot}%{_libdir}/systemd/user/tizen-middleware.target.wants
install %{SOURCE1} %{buildroot}%{_libdir}/systemd/user/
ln -s ../calendar.service %{buildroot}%{_libdir}/systemd/user/tizen-middleware.target.wants/

%post
/sbin/ldconfig

chown :6003 /opt/usr/data/calendar-svc

mkdir -p /opt/usr/dbspace
if [ -f /opt/usr/dbspace/.calendar-svc.db ]
then
        echo "calendar-svc.db exist"
else
		calendar-svc-initdb
fi

if [ -f /usr/lib/rpm-plugins/msm.so ]
then
	chsmack -a 'calendar-service::db' /opt/usr/dbspace/.calendar-svc.db*
fi

chown :6003 /opt/usr/dbspace/.calendar-svc.db
chown :6003 /opt/usr/dbspace/.calendar-svc.db-journal
chown :6003 /opt/usr/data/calendar-svc/.CALENDAR_SVC_*

chmod 660 /opt/usr/dbspace/.calendar-svc.db
chmod 660 /opt/usr/dbspace/.calendar-svc.db-journal
chmod 660 /opt/usr/data/calendar-svc/.CALENDAR_SVC_*

vconftool set -t int db/calendar/timezone_on_off 0 -g 6003
vconftool set -t string db/calendar/timezone_path "Asia/Seoul" -g 6003

%postun -p /sbin/ldconfig

%files
%manifest calendar-service.manifest
%defattr(-,root,root,-)
%{_bindir}/calendar-svc-initdb
%{_libdir}/libcalendar-service-native.so.*
%{_bindir}/calendar-serviced*
%{_libdir}/libcalendar-service2.so.*
%attr(0755,root,root) /etc/rc.d/init.d/calendar-serviced.sh
/etc/rc.d/rc3.d/S85calendar-serviced
/etc/rc.d/rc5.d/S85calendar-serviced
%dir %attr(0775,root,root) /opt/usr/data/calendar-svc/
/opt/usr/data/calendar-svc/.CALENDAR_SVC_CALENDAR_CHANGED
/opt/usr/data/calendar-svc/.CALENDAR_SVC_EVENT_CHANGED
/opt/usr/data/calendar-svc/.CALENDAR_SVC_TODO_CHANGED
/usr/share/calendar-svc/dft-calendar
%{_libdir}/systemd/user/calendar.service
%{_libdir}/systemd/user/tizen-middleware.target.wants/calendar.service

%files devel
%defattr(-,root,root,-)
%{_includedir}/calendar-service-native/*.h
%{_includedir}/calendar-service2/*.h
%{_libdir}/*.so
%{_libdir}/pkgconfig/calendar.pc
%{_libdir}/pkgconfig/calendar-service-native.pc
%{_libdir}/pkgconfig/calendar-service2.pc
