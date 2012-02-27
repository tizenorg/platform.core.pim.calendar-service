Name:	    libslp-calendar
Summary:    DB library for calendar
Version:    0.1.10
Release:    12
Group:      System/Libraries
License:    Apache 2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires: cmake
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(gmodule-2.0)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(alarm-service)
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
mkdir -p /opt/dbspace
if [ -f /opt/dbspace/.calendar-svc.db ]
then
        echo "calendar.db exist"
else
		calendar-svc-initdb
fi

#chown :5000 /opt/dbspace
chown :6003 /opt/dbspace/.calendar-svc.db
chown :6003 /opt/dbspace/.calendar-svc.db-journal
chown :6003 /opt/data/calendar-svc/.CALENDAR_SVC_*

# Change file permissions
#chmod 644 /usr/lib/libcalendar-service.so
#chmod 775 /opt/dbspace
chmod 660 /opt/dbspace/.calendar-svc.db
chmod 660 /opt/dbspace/.calendar-svc.db-journal
chmod 660 /opt/data/calendar-svc/.CALENDAR_SVC_*

%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%attr(0660,root,db_calendar)/opt/data/calendar-svc/.CALENDAR_SVC_*
/usr/lib/libcalendar-service.so.0*
/usr/lib/libcalendar-service.so.0.1.12


%files devel
%defattr(-,root,root,-)
/usr/lib/pkgconfig/calendar-service.pc
/usr/include/calendar/calendar-svc-errors.h
/usr/include/calendar/calendar-svc-provider.h
/usr/include/calendar/calendar-svc-struct.h
/usr/lib/libcalendar-service.so
