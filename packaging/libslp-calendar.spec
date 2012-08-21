Name:       libslp-calendar
Summary:    DB library for calendar
Version:    0.1.13
Release:    36
Group:      System/Libraries
License:    Apache 2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(post): /usr/bin/sqlite3
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


%post
/sbin/ldconfig
mkdir -p /opt/dbspace
if [ -f /opt/dbspace/.calendar-svc.db ]
then
        echo "calendar-svc.db exist"
else
		calendar-svc-initdb
fi

chown :6003 /opt/dbspace/.calendar-svc.db
chown :6003 /opt/dbspace/.calendar-svc.db-journal
chown :6003 /opt/data/calendar-svc/.CALENDAR_SVC_*

chmod 660 /opt/dbspace/.calendar-svc.db
chmod 660 /opt/dbspace/.calendar-svc.db-journal
chmod 660 /opt/data/calendar-svc/.CALENDAR_SVC_*

vconftool set -t int db/calendar/timezone_on_off 0 -g 6003
vconftool set -t string db/calendar/timezone_path "Asia/Seoul" -g 6003

%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%{_bindir}/calendar-svc-initdb
%{_libdir}/libcalendar-service.so.*
/opt/data/calendar-svc/.CALENDAR_SVC_CALENDAR_CHANGED
/opt/data/calendar-svc/.CALENDAR_SVC_EVENT_CHANGED
/opt/data/calendar-svc/.CALENDAR_SVC_TODO_CHANGED

%files devel
%defattr(-,root,root,-)
%{_includedir}/calendar-svc/*.h
%{_includedir}/calendar/*.h
%{_libdir}/*.so
%{_libdir}/pkgconfig/calendar.pc
%{_libdir}/pkgconfig/calendar-service.pc
