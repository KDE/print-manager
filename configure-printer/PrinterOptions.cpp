/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti12@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 *                                                                         *
 *   The save PPD snipet is from CUPS                                      *
 *   Copyright 2007-2009 by Apple Inc.                                     *
 *   Copyright 1997-2007 by Easy Software Products.                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *   This is converted from LGPL 2 in accordance with section 3            *
 *   See http://www.cups.org/documentation.php/license.html                *
 ***************************************************************************/

#include "PrinterOptions.h"

#include "ui_PrinterOptions.h"

#include <KCupsRequest.h>
#include <NoSelectionRectDelegate.h>

#include <QFormLayout>
#include <KComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QStandardItemModel>
#include <QListView>
#include <QGroupBox>

#include <KDebug>

#define DEFAULT_CHOICE "defaultChoice"

PrinterOptions::PrinterOptions(const QString &destName, bool isClass, bool isRemote, QWidget *parent) :
    PrinterPage(parent),
    ui(new Ui::PrinterOptions),
    m_destName(destName),
    m_isClass(isClass),
    m_isRemote(isRemote),
    m_ppd(NULL),
    m_changes(0)
{
    ui->setupUi(this);

    reloadPPD();
}

void PrinterOptions::on_autoConfigurePB_clicked()
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->printCommand(m_destName, "AutoConfigure", i18n("Set Default Options"));
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterOptions::reloadPPD()
{
    //     The caller "owns" the file that is created and must unlink the returned filename.
    if (!m_filename.isNull()) {
        unlink(m_filename.toUtf8());
    }

    // remove all the options
    while (ui->verticalLayout->count()) {
        kDebug() << "removing" << ui->verticalLayout->count();
        QLayoutItem *item = ui->verticalLayout->itemAt(0);
        ui->verticalLayout->removeItem(item);
        if (item->widget()) {
            item->widget()->deleteLater();
            delete item;
        } else if (item->layout()) {
            kDebug() << "removing layout" << ui->verticalLayout->count();

//            item->layout()->deleteLater();
        } else if (item->spacerItem()) {
            delete item->spacerItem();
        }
    }
    m_changes = 0;
    m_customValues.clear();
    emit changed(false);

    QPointer<KCupsRequest> request = new KCupsRequest;
    request->getPrinterPPD(m_destName);
    request->waitTillFinished();
    if (!request) {
        return;
    }
    m_filename = request->printerPPD();
    m_ppd = ppdOpenFile(m_filename.toUtf8());
    request->deleteLater();
    if (m_ppd == NULL) {
        kWarning() << "Could not open ppd file:" << m_filename << request->errorMsg();
        m_filename.clear();
        return;
    }

    // select the default options on the ppd file
    ppdMarkDefaults(m_ppd);

    // TODO tri to use QTextCodec aliases
    const char *lang_encoding;
    lang_encoding = m_ppd->lang_encoding;
    if (lang_encoding && !strcasecmp (lang_encoding, "ISOLatin1")) {
        m_codec = QTextCodec::codecForName("ISO-8859-1");
    } else if (lang_encoding && !strcasecmp (lang_encoding, "ISOLatin2")) {
        m_codec = QTextCodec::codecForName("ISO-8859-2");
    } else if (lang_encoding && !strcasecmp (lang_encoding, "ISOLatin5")) {
        m_codec = QTextCodec::codecForName("ISO-8859-5");
    } else if (lang_encoding && !strcasecmp (lang_encoding, "JIS83-RKSJ")) {
        m_codec = QTextCodec::codecForName("SHIFT-JIS");
    } else if (lang_encoding && !strcasecmp (lang_encoding, "MacStandard")) {
        m_codec = QTextCodec::codecForName("MACINTOSH");
    } else if (lang_encoding && !strcasecmp (lang_encoding, "WindowsANSI")) {
        m_codec = QTextCodec::codecForName("WINDOWS-1252");
    } else {
        // Guess
        m_codec = QTextCodec::codecForName(lang_encoding);
    }
    if (m_codec == 0) {
        m_codec = QTextCodec::codecForName("UTF-8");
    }

    if (m_ppd->manufacturer) {
        m_make = m_codec->toUnicode(m_ppd->manufacturer);
    }

    if (m_ppd->nickname) {
        m_makeAndModel = m_codec->toUnicode(m_ppd->nickname);
    }

    ui->autoConfigurePB->hide();
    ppd_attr_t  *ppdattr;
    if (m_ppd->num_filters == 0 ||
        ((ppdattr = ppdFindAttr(m_ppd, "cupsCommands", NULL)) != NULL &&
           ppdattr->value && strstr(ppdattr->value, "AutoConfigure"))) {
        ui->autoConfigurePB->show();
    } else {
        for (int i = 0; i < m_ppd->num_filters; i ++) {
            if (!strncmp(m_ppd->filters[i], "application/vnd.cups-postscript", 31)) {
              ui->autoConfigurePB->show();
              break;
            }
        }
    }

    createGroups();
}

void PrinterOptions::createGroups()
{
    int i;
    ppd_group_t *group;
    // Iterate over the groups
    for (i = 0, group = m_ppd->groups;
         i < m_ppd->num_groups;
         i++, group++) {
        // The name of the group
        QString name = m_codec->toUnicode(group->name);

        // The humman name of the group
        QString text = m_codec->toUnicode(group->text);

        // The group box were the options will be laid out
        QGroupBox *groupBox = new QGroupBox(text, ui->scrollArea);

        // Create the form layout to put options in
        QFormLayout *gFormLayout = new QFormLayout(groupBox);
        gFormLayout->setFormAlignment(Qt::AlignCenter);
        groupBox->setLayout(gFormLayout);
        ui->verticalLayout->addWidget(groupBox);

        int j;
        ppd_option_t *option;
        // Iterate over the options in the group
        for (j = 0, option = group->options;
             j < group->num_options;
             j++, option++) {
            QString oKeyword = m_codec->toUnicode(option->keyword);
            QString oText = m_codec->toUnicode(option->text);
            QString oDefChoice = m_codec->toUnicode(option->defchoice);
            // The python system-config-printer skips this one
            // which has the same data as "PageSize", let's hope
            // they did the right thing
            if (oKeyword == "PageRegion") {
                continue;
            }

            QWidget *optionW = 0;
            switch (option->ui) {
            case PPD_UI_BOOLEAN:
                optionW = pickBoolean(option, oKeyword, ui->scrollAreaWidgetContents);
                break;
            case PPD_UI_PICKMANY:
                optionW = pickMany(option, oKeyword, ui->scrollAreaWidgetContents);
                break;
            case PPD_UI_PICKONE:
                optionW = pickOne(option, oKeyword, ui->scrollAreaWidgetContents);
                break;
            default:
                kWarning() << "Option type not recognized: " << option->ui;
                // let's use the most common
                optionW = pickOne(option, oKeyword, ui->scrollAreaWidgetContents);
                break;
            }

            if (optionW) {
                // insert the option widget
                gFormLayout->addRow(oText, optionW);
            }
        }
    }
    ui->verticalLayout->addStretch();
}

QWidget* PrinterOptions::pickBoolean(ppd_option_t *option, const QString &keyword, QWidget *parent) const
{
    Q_UNUSED(keyword)
    QWidget *widget = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    QButtonGroup *radioGroup = new QButtonGroup(widget);
    widget->setLayout(layout);

    int i;
    ppd_choice_t *choice;
    QString defChoice = m_codec->toUnicode(option->defchoice);
    // Iterate over the choices in the option
    for (i = 0, choice = option->choices;
         i < option->num_choices;
         ++i, ++choice) {
        QString choiceName = m_codec->toUnicode(choice->choice);
        QString cText = m_codec->toUnicode(choice->text);

        QRadioButton *button = new QRadioButton(cText, widget);
        button->setChecked(defChoice == choiceName);
        button->setProperty("choice", choiceName);
        // if we are in looking at a remote printer we can't save it
        button->setEnabled(!m_isRemote);
        layout->addWidget(button);
        radioGroup->addButton(button);
    }

    // store the default choice
    radioGroup->setProperty(DEFAULT_CHOICE, defChoice);
    radioGroup->setProperty("Keyword", keyword);
    connect(radioGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(radioBtClicked(QAbstractButton*)));
    return widget;
}

void PrinterOptions::radioBtClicked(QAbstractButton *button)
{
    QObject *radioGroup = sender();
    bool isDifferent = radioGroup->property(DEFAULT_CHOICE).toString() != button->property("choice");

    if (isDifferent != radioGroup->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        radioGroup->setProperty("different", isDifferent);
        emit changed(m_changes);
    }

    QString keyword = radioGroup->property("Keyword").toString();
    QString choice = button->property("choice").toString();
    radioGroup->setProperty("currentChoice", choice);

    // TODO warning about conflicts
//     ppdMarkOption(m_ppd,
//                   m_codec->fromUnicode(keyword),
//                   m_codec->fromUnicode(choice));
    // store the new value
    if (isDifferent) {
        m_customValues[keyword] = radioGroup;
    } else {
        m_customValues.remove(keyword);
    }
}

QWidget* PrinterOptions::pickMany(ppd_option_t *option, const QString &keyword, QWidget *parent) const
{
    Q_UNUSED(keyword)
    QListView *listView = new QListView(parent);
    QStandardItemModel *model = new QStandardItemModel(listView);
    listView->setModel(model);
    listView->setItemDelegate(new NoSelectionRectDelegate(listView));

    int i;
    ppd_choice_t *choice;
    QString oDefChoice = m_codec->toUnicode(option->defchoice);
    // Iterate over the choices in the option
    for (i = 0, choice = option->choices;
         i < option->num_choices;
         ++i, ++choice) {
        QString cName = m_codec->toUnicode(choice->choice);
        QString cText = m_codec->toUnicode(choice->text);

        QStandardItem *item = new QStandardItem(cText);
        item->setData(cName);
        item->setCheckable(true);
        item->setEditable(false);
        // TODO there is only ONE default choice, what about the other
        // Items selected?!
        item->setCheckState(oDefChoice == cName ? Qt::Checked : Qt::Unchecked);
        model->appendRow(item);
    }
    // if we are in looking at a remote printer we can't save it
    listView->setEnabled(!m_isRemote);
    return qobject_cast<QWidget*>(listView);
}

QWidget* PrinterOptions::pickOne(ppd_option_t *option, const QString &keyword, QWidget *parent) const
{
    int i;
    ppd_choice_t *choice;
    QString defChoice = m_codec->toUnicode(option->defchoice);
    KComboBox *comboBox = new KComboBox(parent);
    // Iterate over the choices in the option
    for (i = 0, choice = option->choices;
         i < option->num_choices;
         ++i, ++choice) {
        QString cName = m_codec->toUnicode(choice->choice);
        QString cText = m_codec->toUnicode(choice->text);

        comboBox->addItem(cText, cName);
    }
    // store the default choice
    comboBox->setProperty(DEFAULT_CHOICE, defChoice);
    comboBox->setProperty("Keyword", keyword);
    comboBox->setCurrentIndex(comboBox->findData(defChoice));
    // connect the signal AFTER setCurrentIndex is called
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChangedCB(int)));
    // if we are in looking at a remote printer we can't save it
    comboBox->setEnabled(!m_isRemote);
    return qobject_cast<QWidget*>(comboBox);
}

void PrinterOptions::currentIndexChangedCB(int index)
{
    KComboBox *comboBox = qobject_cast<KComboBox*>(sender());
    bool isDifferent = comboBox->property(DEFAULT_CHOICE).toString() != comboBox->itemData(index);

    if (isDifferent != comboBox->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        comboBox->setProperty("different", isDifferent);
        emit changed(m_changes);
    }

    QString keyword = comboBox->property("Keyword").toString();
    QString value = comboBox->itemData(index).toString();
    comboBox->setProperty("currentChoice", value);

    // TODO warning about conflicts
//     ppdMarkOption(m_ppd,
//                   m_codec->fromUnicode(keyword),
//                   m_codec->fromUnicode(value));
    // store the new value
    if (isDifferent) {
        m_customValues[keyword] = qobject_cast<QObject*>(comboBox);
    } else {
        m_customValues.remove(keyword);
    }
}

PrinterOptions::~PrinterOptions()
{
    if (m_ppd != NULL) {
        ppdClose(m_ppd);
    }

    if (!m_filename.isNull()) {
        unlink(m_filename.toUtf8());
    }

    delete ui;
}

const char *                            /* O - Value of variable */
PrinterOptions::getVariable(const char *name)    const    /* I - Name of variable */
{
    QString keyword = m_codec->toUnicode(name);
    if (m_customValues.contains(keyword)) {
        QString value = m_customValues[keyword]->property("currentChoice").toString();
        return m_codec->fromUnicode(value);
    } else {
        return NULL;
    }
}

/*
 * 'get_points()' - Get a value in points.
 */
double                           /* O - Number in points */
PrinterOptions::get_points(double     number,           /* I - Original number */
           const char *uval)            /* I - Units */
{
    if (!strcmp(uval, "mm"))              /* Millimeters */
        return (number * 72.0 / 25.4);
    else if (!strcmp(uval, "cm"))         /* Centimeters */
        return (number * 72.0 / 2.54);
    else if (!strcmp(uval, "in"))         /* Inches */
        return (number * 72.0);
    else if (!strcmp(uval, "ft"))         /* Feet */
        return (number * 72.0 * 12.0);
    else if (!strcmp(uval, "m"))          /* Meters */
        return (number * 72.0 / 0.0254);
    else                                  /* Points */
        return (number);
}

/*
 * 'get_option_value()' - Return the value of an option.
 *
 * This function also handles generation of custom option values.
 */

char *                           /* O - Value string or NULL on error */
PrinterOptions::get_option_value(
    ppd_file_t    *ppd,                 /* I - PPD file */
    const char    *name,                /* I - Option name */
    char          *buffer,              /* I - String buffer */
    size_t        bufsize) const             /* I - Size of buffer */
{
    char          *bufptr,                /* Pointer into buffer */
            *bufend;                /* End of buffer */
    ppd_coption_t *coption;               /* Custom option */
    ppd_cparam_t  *cparam;                /* Current custom parameter */
    char          keyword[256];           /* Parameter name */
    const char    *val,                   /* Parameter value */
            *uval;                  /* Units value */
    long          integer;                /* Integer value */
    double        number,                 /* Number value */
            number_points;          /* Number in points */


    /*
     * See if we have a custom option choice...
     */

    if ((val = getVariable(name)) == NULL) {
        /*
         * Option not found!
         */

        return (NULL);
    } else if (strcasecmp(val, "Custom") ||
               (coption = ppdFindCustomOption(ppd, name)) == NULL) {
        /*
         * Not a custom choice...
         */

        qstrncpy(buffer, val, bufsize);
        return (buffer);
    }

    /*
     * OK, we have a custom option choice, format it...
     */

    *buffer = '\0';

    if (!strcmp(coption->keyword, "PageSize")) {
        const char  *lval;                  /* Length string value */
        double      width,                  /* Width value */
                    width_points,           /* Width in points */
                    length,                 /* Length value */
                    length_points;          /* Length in points */


        val  = getVariable("PageSize.Width");
        lval = getVariable("PageSize.Height");
        uval = getVariable("PageSize.Units");

        if (!val || !lval || !uval ||
                (width = strtod(val, NULL)) == 0.0 ||
                (length = strtod(lval, NULL)) == 0.0 ||
                (strcmp(uval, "pt") && strcmp(uval, "in") && strcmp(uval, "ft") &&
                 strcmp(uval, "cm") && strcmp(uval, "mm") && strcmp(uval, "m"))) {
            return (NULL);
        }

        width_points  = get_points(width, uval);
        length_points = get_points(length, uval);

        if (width_points < ppd->custom_min[0] ||
                width_points > ppd->custom_max[0] ||
                length_points < ppd->custom_min[1] ||
                length_points > ppd->custom_max[1]) {
            return (NULL);
        }

        snprintf(buffer, bufsize, "Custom.%gx%g%s", width, length, uval);
    } else if (cupsArrayCount(coption->params) == 1) {
        cparam = ppdFirstCustomParam(coption);
        snprintf(keyword, sizeof(keyword), "%s.%s", coption->keyword, cparam->name);

        if ((val = getVariable(keyword)) == NULL)
            return (NULL);

        switch (cparam->type) {
        case PPD_CUSTOM_CURVE :
        case PPD_CUSTOM_INVCURVE :
        case PPD_CUSTOM_REAL :
            if ((number = strtod(val, NULL)) == 0.0 ||
                    number < cparam->minimum.custom_real ||
                    number > cparam->maximum.custom_real)
                return (NULL);

            snprintf(buffer, bufsize, "Custom.%g", number);
            break;
        case PPD_CUSTOM_INT :
            if (!*val || (integer = strtol(val, NULL, 10)) == LONG_MIN ||
                    integer == LONG_MAX ||
                    integer < cparam->minimum.custom_int ||
                    integer > cparam->maximum.custom_int)
                return (NULL);

            snprintf(buffer, bufsize, "Custom.%ld", integer);
            break;
        case PPD_CUSTOM_POINTS :
            snprintf(keyword, sizeof(keyword), "%s.Units", coption->keyword);

            if ((number = strtod(val, NULL)) == 0.0 ||
                    (uval = getVariable(keyword)) == NULL ||
                    (strcmp(uval, "pt") && strcmp(uval, "in") && strcmp(uval, "ft") &&
                     strcmp(uval, "cm") && strcmp(uval, "mm") && strcmp(uval, "m")))
                return (NULL);

            number_points = get_points(number, uval);
            if (number_points < cparam->minimum.custom_points ||
                    number_points > cparam->maximum.custom_points)
                return (NULL);

            snprintf(buffer, bufsize, "Custom.%g%s", number, uval);
            break;
        case PPD_CUSTOM_PASSCODE :
            for (uval = val; *uval; ++uval) {
                if (!isdigit(*uval & 255)) {
                    return (NULL);
                }
            }
        case PPD_CUSTOM_PASSWORD :
        case PPD_CUSTOM_STRING :
            integer = (long)strlen(val);
            if (integer < cparam->minimum.custom_string ||
                    integer > cparam->maximum.custom_string) {
                return (NULL);
            }

            snprintf(buffer, bufsize, "Custom.%s", val);
            break;
        }
    } else {
        const char *prefix = "{";           /* Prefix string */


        bufptr = buffer;
        bufend = buffer + bufsize;

        for (cparam = ppdFirstCustomParam(coption);
             cparam;
             cparam = ppdNextCustomParam(coption)) {
            snprintf(keyword, sizeof(keyword), "%s.%s", coption->keyword,
                     cparam->name);

            if ((val = getVariable(keyword)) == NULL) {
                return (NULL);
            }

            snprintf(bufptr, bufend - bufptr, "%s%s=", prefix, cparam->name);
            bufptr += strlen(bufptr);
            prefix = " ";

            switch (cparam->type) {
            case PPD_CUSTOM_CURVE :
            case PPD_CUSTOM_INVCURVE :
            case PPD_CUSTOM_REAL :
                if ((number = strtod(val, NULL)) == 0.0 ||
                        number < cparam->minimum.custom_real ||
                        number > cparam->maximum.custom_real)
                    return (NULL);

                snprintf(bufptr, bufend - bufptr, "%g", number);
                break;
            case PPD_CUSTOM_INT :
                if (!*val || (integer = strtol(val, NULL, 10)) == LONG_MIN ||
                        integer == LONG_MAX ||
                        integer < cparam->minimum.custom_int ||
                        integer > cparam->maximum.custom_int) {
                    return (NULL);
                }

                snprintf(bufptr, bufend - bufptr, "%ld", integer);
                break;
            case PPD_CUSTOM_POINTS :
                snprintf(keyword, sizeof(keyword), "%s.Units", coption->keyword);

                if ((number = strtod(val, NULL)) == 0.0 ||
                        (uval = getVariable(keyword)) == NULL ||
                        (strcmp(uval, "pt") && strcmp(uval, "in") &&
                         strcmp(uval, "ft") && strcmp(uval, "cm") &&
                         strcmp(uval, "mm") && strcmp(uval, "m"))) {
                    return (NULL);
                }

                number_points = get_points(number, uval);
                if (number_points < cparam->minimum.custom_points ||
                        number_points > cparam->maximum.custom_points) {
                    return (NULL);
                }

                snprintf(bufptr, bufend - bufptr, "%g%s", number, uval);
                break;

            case PPD_CUSTOM_PASSCODE :
                for (uval = val; *uval; uval ++) {
                    if (!isdigit(*uval & 255)) {
                        return (NULL);
                    }
                }
            case PPD_CUSTOM_PASSWORD :
            case PPD_CUSTOM_STRING :
                integer = (long)strlen(val);
                if (integer < cparam->minimum.custom_string ||
                        integer > cparam->maximum.custom_string) {
                    return (NULL);
                }

                if ((bufptr + 2) > bufend) {
                    return (NULL);
                }

                bufend --;
                *bufptr++ = '\"';

                while (*val && bufptr < bufend) {
                    if (*val == '\\' || *val == '\"') {
                        if ((bufptr + 1) >= bufend) {
                            return (NULL);
                        }

                        *bufptr++ = '\\';
                    }

                    *bufptr++ = *val++;
                }

                if (bufptr >= bufend) {
                    return (NULL);
                }

                *bufptr++ = '\"';
                *bufptr   = '\0';
                bufend ++;
                break;
            }

            bufptr += strlen(bufptr);
        }

        if (bufptr == buffer || (bufend - bufptr) < 2) {
            return (NULL);
        }

        strcpy(bufptr, "}");
    }

    return (buffer);
}


void PrinterOptions::save()
{
    char tempfile[1024];
    const char  *var;
    cups_file_t *in,                    /* Input file */
                *out;                   /* Output file */
    char        line[1024],             /* Line from PPD file */
                value[1024],            /* Option value */
                keyword[1024],          /* Keyword from Default line */
                *keyptr;                /* Pointer into keyword... */

    // copy cups-1.4.2/cgi-bin line 3779
    if (!m_filename.isNull()) {
        out = cupsTempFile2(tempfile, sizeof(tempfile));
        in  = cupsFileOpen(m_filename.toUtf8(), "r");

        if (!in || !out)
        {
            if (in) {
                cupsFileClose(in);
            }

            if (out) {
                cupsFileClose(out);
                unlink(tempfile);
            }

            // TODO add a KMessageBox::error

            return;
        }

        while (cupsFileGets(in, line, sizeof(line))) {
            if (!strncmp(line, "*cupsProtocol:", 14)) {
                continue;
            } else if (strncmp(line, "*Default", 8)) {
                cupsFilePrintf(out, "%s\n", line);
            } else {
                /*
                * Get default option name...
                */
                qstrncpy(keyword, line + 8, sizeof(keyword));

                for (keyptr = keyword; *keyptr; keyptr ++) {
                    if (*keyptr == ':' || isspace(*keyptr & 255)) {
                        break;
                    }
                }

                *keyptr = '\0';

                if (!strcmp(keyword, "PageRegion") ||
                    !strcmp(keyword, "PaperDimension") ||
                    !strcmp(keyword, "ImageableArea")) {
                    var = get_option_value(m_ppd, "PageSize", value, sizeof(value));
                } else {
                    var = get_option_value(m_ppd, keyword, value, sizeof(value));
                }

                if (!var) {
                    cupsFilePrintf(out, "%s\n", line);
                } else {
                    cupsFilePrintf(out, "*Default%s: %s\n", keyword, var);
                }
            }
        }

        cupsFileClose(in);
        cupsFileClose(out);
    }

    QVariantHash values; // we need null values
    QPointer<KCupsRequest> request = new KCupsRequest;
    if (m_isClass) {
        request->addOrModifyClass(m_destName, values);
    } else {
        request->addOrModifyPrinter(m_destName, values, tempfile);
    }

    // Disable the widget till the request is processed
    // Otherwise the user might change something in the ui
    // which won't be saved but the apply but when the request
    // finishes we will set the current options as default
    setEnabled(false);
    request->waitTillFinished();

    // unlink the file
    unlink(tempfile);

    if (request) {
        setEnabled(true);
        if (!request->hasError()) {
            // if we succefully save the new ppd we need now to
            // clear our changes
            QHash<QString, QObject*>::const_iterator i = m_customValues.constBegin();
            while (i != m_customValues.constEnd()) {
                QString currentChoice;
                currentChoice = i.value()->property("currentChoice").toString();
                // Store the current choice as the default one
                i.value()->setProperty(DEFAULT_CHOICE, currentChoice);
                i.value()->setProperty("currentChoice", QVariant());
                i.value()->setProperty("different", false);
                ++i;
            }
            m_changes = 0;
            m_customValues.clear();
            emit changed(false);
        }
        request->deleteLater();
    }
}

bool PrinterOptions::hasChanges()
{
    return m_changes;
}

QString PrinterOptions::currentMake() const
{
    return m_make;
}

QString PrinterOptions::currentMakeAndModel() const
{
    return m_makeAndModel;
}

#include "PrinterOptions.moc"
