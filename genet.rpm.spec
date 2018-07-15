%define        __spec_install_post %{nil}
%define          debug_package %{nil}
%define        __os_install_post %{_dbpath}/brp-compress

Summary: General Purpose Network Analyzer
Name: genet
Version: {{GENET_VERSION}}
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

/usr/share/genet/
/usr/share/applications/genet.desktop
/usr/share/doc/genet/copyright
/usr/share/icons/hicolor/256x256/apps/genet.png
/usr/share/lintian/overrides/genet


%changelog

%post
setcap cap_net_raw,cap_net_admin=p /usr/share/genet/genet
