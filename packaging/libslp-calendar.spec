Name:	    libslp-calendar
Summary:    DB library for calendar
Version:    0.1.12
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires: cmake
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(gconf-2.0)
BuildRequires: pkgconfig(gmodule-2.0)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(heynoti)
BuildRequires: pkgconfig(alarm-service)
BuildRequires: pkgconfig(aul)
Requires(post): /usr/bin/sqlite3

%description
DB library for calendar

%package devel
Summary:    DB library for calendar
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

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

/usr/bin/calendar-svc-initdb

chown root:root /usr/lib/libcalendar-service.so.*
chown :6003 /opt/data/calendar-svc/.CALENDAR_SVC_*
chown :6003 /opt/dbspace/.calendar-svc.db
chown :6003 /opt/dbspace/.calendar-svc.db-journal

chmod 660 /opt/dbspace/.calendar-svc.db
chmod 660 /opt/dbspace/.calendar-svc.db-journal
chmod 660 /opt/data/calendar-svc/.CALENDAR_SVC_*


%postun -p /sbin/ldconfig


%files
/usr/lib/lib*.so.*
/usr/bin/calendar-svc-initdb
/opt/data/calendar-svc/.*

%files devel
/usr/lib/pkgconfig/*.pc
/usr/include/calendar/*.h
/usr/include/calendar-svc/*.h
/usr/lib/lib*.so
