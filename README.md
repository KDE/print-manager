# Print Manager

Print Manager is a small, relatively self-contained set of components integrated with Plasma System Settings for managing CUPS printer configurations.


## Components

* Legacy stand-alone app:
  * `configure-printer` manages print job media options and other settings

* Print job manager
  * `plasma-print-queue` provides a single interface for managing jobs in
    configured print queues

* System tray plasmoid provides quick access to printers and print job status

* KDed module for new device and ink/toner level notifications

* KCM module for print queue setup and configuration

* KCups Library provides an interface to the CUPS api as well as a few models that are used by the legacy apps, the KCM and the plasmoid


## General

The print-manager KCM provides the primary means to add/configure printers and printer groups.  In addition, some of the basic CUPS print server options can be configured.  There are some limitations to the KCM that mirror the (pre-Plasma6) legacy apps:

* Limited support for Windows printers using samba
* Limited support for auto print queue identification for older connection schemes (LPD/SOCKET)

The KCM also depends on the following features that are present in the legacy 
`configure-printer` app:

* Media settings (paper size, mode, type, quality, etc)
* Banners, Policies and Allowed Users

These features are available via the `configure-printer` app, accessed from the KCM or the print queue manager.  In a future release, these features will be integrated with the KCM.


## Building, Running and Testing

The easiest way to make changes and test print-manager during development is to [build it with kde-builder](https://kde-builder.kde.org).

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


## Contributing

Like other projects in the KDE ecosystem, contributions are welcome from all. This repository is managed in [KDE Invent](https://invent.kde.org/plasma/print-manager), our GitLab instance.

* Want to contribute code? See the [GitLab wiki page](https://community.kde.org/Infrastructure/GitLab) for a tutorial on how to send a merge request.
* Reporting a bug? Please submit it on the KDE Bugtracking System, for [bugs in the KCM](https://bugs.kde.org/buglist.cgi?product=systemsettings&component=kcm_printer_manager&resolution=---) or for [bugs in the Printers widget](https://bugs.kde.org/buglist.cgi?product=plasmashell&component=Printers%20widget&resolution=---). Please do not use the Issues tab in GitLab to report bugs.
* Is there a part of print-manager that's not translated? See the [Getting Involved in Translation wiki page](https://community.kde.org/Get_Involved/translation) to see how you can help translate!

If you get stuck or need help with anything at all, head over to the [KDE New Contributors room](https://go.kde.org/matrix/#/#kde-welcome:kde.org) on Matrix. For questions about print-manager, please ask in the [KDE Printing Development room](https://go.kde.org/matrix/#/#kde-printing-dev:kde.org). See [Matrix](https://community.kde.org/Matrix) for more details.
