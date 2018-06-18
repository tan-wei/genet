%define        __spec_install_post %{nil}
%define          debug_package %{nil}
%define        __os_install_post %{_dbpath}/brp-compress

Summary: Next generation packet analyzer
Name: deplug
Version: {{DEPLUG_VERSION}}
Release: 1
License: MIT
Requires: libcap1
Requires(post): libcap-progs
AutoReqProv: no

%description
Genet is a modern packet analyzer based on Electron.

%prep

%build

%install
rm -rf %{buildroot}
mkdir -p  %{buildroot}
cp -a * %{buildroot}

%clean

%files
%defattr(-,root,root)
%doc

/usr/share/deplug/
/usr/share/applications/deplug.desktop
/usr/share/doc/deplug/copyright
/usr/share/icons/hicolor/256x256/apps/deplug.png
/usr/share/lintian/overrides/deplug


%changelog

%post
setcap cap_net_raw,cap_net_admin=p /usr/share/deplug/deplug
