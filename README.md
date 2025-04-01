# Print Manager

Print Manager is a small, relatively self-contained set of components integrated with Plasma System Settings for managing CUPS printer configurations.


## Components

* Set of legacy stand-alone apps:
  * `configure-printer` (for configuring current printers and printer groups)
  * `kde-print-queue` (for managing printer job queues)

* System tray plasmoid that provides quick access to printers and print job status

* KDed module for new device and ink/toner level notifications

* KCM module 

* KCups Library that provides an interface to the CUPS api as well as a few models that are used by the legacy apps, the KCM and the plasmoid


## General

For Plasma6, the KCM will provide the primary means to add/configure printers and printer groups.  In addition, some of the basic CUPS print server options can be configured.  There are some limitations to KCM add-printer features that mirror the legacy apps:

* Limited support for Windows printers using samba
* Limited support for auto print queue identification for older connection schemes (LPD/SOCKET)

The KCM also depends on the following features that are present in the legacy `configure-printer` app:

* Media settings (paper size, mode, type, quality, etc)
* Banners, Policies and Allowed Users
  
These features are available via the `configure-printer` app, accessed from the KCM and the plasmoid as a configure action.  In a future release, these features will be integrated with the KCM.

The legacy `kde-print-queue` app is provided as the primary means to manage configured printer job queues.  It remains directly accessible from both the KCM and the plasmoid.


## Building and Running

The easiest way to make changes and test print-manager during development is to [build it with kde-builder](https://kde-builder.kde.org).

Enable BUILD_TESTING=on to build the tests.

After building, restart the kded and plasmashell services.  Run the kcm from System Settings or from the command line:

* systemsettings kcm_printer_manager
* kcmshell6 kcm_printer_manager

The following command line options are supported (with the --args parameter)

* --add-printer
* --add-group
* --configure-printer <queueName>

For example, to add a printer run: 

* `systemsettings kcm_printer_manager --args --add-printer`

To configure an existing printer, run: 

* `systemsettings kcm_printer_manager --args "--configure-printer <queueName>"`


## Development Testing

Tests can be run locally, manually after building (see build/bin folder).  The dbus exercise tests (kded) must be run with elevated privileges because the dbus calls are on the system bus.  This emulates the CUPS applet behavior which uses the system bus to notify new printer events.


## Contributing

Like other projects in the KDE ecosystem, contributions are welcome from all. This repository is managed in [KDE Invent](https://invent.kde.org/plasma/print-manager), our GitLab instance.

* Want to contribute code? See the [GitLab wiki page](https://community.kde.org/Infrastructure/GitLab) for a tutorial on how to send a merge request.
* Reporting a bug? Please submit it on the KDE Bugtracking System, for [bugs in the KCM](https://bugs.kde.org/buglist.cgi?product=systemsettings&component=kcm_printer_manager&resolution=---) or for [bugs in the Printers widget](https://bugs.kde.org/buglist.cgi?product=plasmashell&component=Printers%20widget&resolution=---). Please do not use the Issues tab in GitLab to report bugs.
* Is there a part of print-manager that's not translated? See the [Getting Involved in Translation wiki page](https://community.kde.org/Get_Involved/translation) to see how
you can help translate!

If you get stuck or need help with anything at all, head over to the [KDE New Contributors room](https://go.kde.org/matrix/#/#kde-welcome:kde.org) on Matrix. For questions about print-manager, please ask in the [KDE Printing Development room](https://go.kde.org/matrix/#/#kde-printing-dev:kde.org). See [Matrix](https://community.kde.org/Matrix) for more details.
