[![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://gitlab.xfce.org/apps/xfce4-screenshooter/-/blob/master/COPYING)

**NOTE:** This is a custom xfce4-screenshooter build that uploads images to 0x0.st like hosts, instead of imgur. To change the host location, you can edit the `upload_url` variable in lib/screenshooter-imgur.c.

# xfce4-screenshooter

Xfce4-screenshooter allows you to capture the entire screen, the active
window or a selected region. You can set the delay that elapses
before the screenshot is taken and the action that will be done with
the screenshot: save it to a /PNG file, copy it to the clipboard, open
it using another application, or host it on imgur.com,
a free online image hosting service.

----

### Homepage

[Xfce4-screenshooter documentation](https://docs.xfce.org/apps/xfce4-screenshooter/start)

### Changelog

See [NEWS](https://gitlab.xfce.org/apps/xfce4-screenshooter/-/blob/master/NEWS) for details on changes and fixes made in the current release.

### Source Code Repository

[Xfce4-screenshooter source code](https://gitlab.xfce.org/apps/xfce4-screenshooter)

### Download a Release Tarball

[Xfce4-screenshooter archive](https://archive.xfce.org/src/apps/xfce4-screenshooter)
    or
[Xfce4-screenshooter tags](https://gitlab.xfce.org/apps/xfce4-screenshooter/-/tags)

### Installation

From source code repository: 

    % cd xfce4-screenshooter
    % ./autogen.sh
    % make
    % make install

From release tarball:

    % tar xf xfce4-screenshooter-<version>.tar.bz2
    % cd xfce4-screenshooter-<version>
    % ./configure
    % make
    % make install

### Reporting Bugs

Visit the [reporting bugs](https://docs.xfce.org/apps/xfce4-screenshooter/bugs) page to view currently open bug reports and instructions on reporting new bugs or submitting bugfixes.

