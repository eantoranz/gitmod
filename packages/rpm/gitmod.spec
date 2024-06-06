Name:           gitmod
Version:        0.10
Release:        1%{?dist}
Summary:        fuse-based linux kernel module to display a committish from a git repo on a mount point.

License:        GPLv2
URL:            https://github.com/eantoranz/gitmod
Source0:        https://github.com/eantoranz/gitmod

BuildRequires:  fuse3-devel, libgit2-devel, glib2-devel
Requires:       fuse3, libgit2, glib2

%description
fuse-based linux kernel module to display a treeish from a git repo
 onto a mount point. The content displayed on the file system is read-only.

%prep
%autosetup


%build
%make_build


%install
%make_install prefix=/usr


%files
%{_bindir}/*


%changelog
* Wed Jun 05 2024 Edmundo Carmona Antoranz
- Initial release using v0.10
