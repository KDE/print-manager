/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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
 ***************************************************************************/

#include "PrinterOptions.h"

#include <cups/cups.h>

#include <QScrollArea>
#include <QFormLayout>
#include <QComboBox>
#include <QRadioButton>
#include <QStandardItemModel>
#include <QListView>
#include <KDebug>

using namespace QCups;

PrinterOptions::PrinterOptions(const QString &destName, QWidget *parent)
 : PrinterPage(parent), m_ppd(NULL), m_changes(0)
{
    setupUi(this);

    m_ppd = ppdOpenFile(cupsGetPPD(destName.toUtf8()));
    if (m_ppd == NULL) {
        kWarning() << "Could not open ppd file";
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

    createGroups();

//     makeCB->addItem(m_printer->value("printer-make-and-model"));
//     nameLE->setText(m_printer->value("printer-info"));
//     nameLE->setProperty("orig_text", m_printer->value("printer-info"));
//     locationLE->setText(m_printer->value("printer-location"));
//     locationLE->setProperty("orig_text", m_printer->value("printer-location"));
//     connectionLE->setText(m_printer->value("device-uri"));
//     connectionLE->setProperty("orig_text", m_printer->value("device-uri"));
//     connect(nameLE, SIGNAL(textChanged(const QString &)),
//             this, SLOT(textChanged(const QString &)));
//     connect(locationLE, SIGNAL(textChanged(const QString &)),
//             this, SLOT(textChanged(const QString &)));
//     connect(connectionLE, SIGNAL(textChanged(const QString &)),
//             this, SLOT(textChanged(const QString &)));
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

        // Create the ScrollArea to put options in
        QScrollArea *scrollArea = new QScrollArea(this);
        QWidget *scrollAreaWidgetContents = new QWidget(scrollArea);
        scrollArea->setWidget(scrollAreaWidgetContents);
        scrollArea->setFrameShape(QFrame::NoFrame);

        QFormLayout *gFormLayout = new QFormLayout(scrollAreaWidgetContents);
        gFormLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
        scrollAreaWidgetContents->setLayout(gFormLayout);
//         gFormLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        gFormLayout->setSizeConstraint(QLayout::SetMinimumSize);

        m_groupsTab[name] = optionsTW->addTab(scrollArea, text);

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
                optionW = pickBoolean(option, scrollAreaWidgetContents);
                break;
            case PPD_UI_PICKMANY:
                optionW = pickMany(option, scrollAreaWidgetContents);
                break;
            case PPD_UI_PICKONE:
                optionW = pickOne(option, scrollAreaWidgetContents);
                break;
            default:
                kWarning() << "Option type not recognized: " << option->ui;
                // let's use the most common
                optionW = pickOne(option, scrollAreaWidgetContents);
                break;
            }

            if (optionW) {
                // insert the option widget
                gFormLayout->addRow(oText, optionW);
            }
//         kDebug() << oKeyword << oText << option->ui << oDefChoice;
        }
//         kDebug() << name << text;
  }
}

QWidget* PrinterOptions::pickBoolean(ppd_option_t *option, QWidget *parent) const
{
    QWidget *widget = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    widget->setLayout(layout);

    int i;
    ppd_choice_t *choice;
    QString oDefChoice = m_codec->toUnicode(option->defchoice);
    // Iterate over the choices in the option
    for (i = 0, choice = option->choices;
         i < option->num_choices;
         i++, choice++) {
        QString cName = m_codec->toUnicode(choice->choice);
        QString cText = m_codec->toUnicode(choice->text);

        QRadioButton *button = new QRadioButton(cText, widget);
        button->setChecked(oDefChoice == cName);
        layout->addWidget(button);
    }
    return widget;
}

QWidget* PrinterOptions::pickMany(ppd_option_t *option, QWidget *parent) const
{
    QListView *listView = new QListView(parent);
    QStandardItemModel *model = new QStandardItemModel(listView);
    listView->setModel(model);

    int i;
    ppd_choice_t *choice;
    QString oDefChoice = m_codec->toUnicode(option->defchoice);
    // Iterate over the choices in the option
    for (i = 0, choice = option->choices;
         i < option->num_choices;
         i++, choice++) {
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
    return qobject_cast<QWidget*>(listView);
}

QWidget* PrinterOptions::pickOne(ppd_option_t *option, QWidget *parent) const
{
    int i;
    ppd_choice_t *choice;
    QString oDefChoice = m_codec->toUnicode(option->defchoice);
    QComboBox *comboBox = new QComboBox(parent);
    // Iterate over the choices in the option
    for (i = 0, choice = option->choices;
         i < option->num_choices;
         i++, choice++) {
        QString cName = m_codec->toUnicode(choice->choice);
        QString cText = m_codec->toUnicode(choice->text);

        comboBox->addItem(cText, cName);
    }
    // selects the default choice
    int defaultChoice = comboBox->findData(oDefChoice);
    comboBox->setProperty("defaultChoice", defaultChoice);
    comboBox->setCurrentIndex(comboBox->findData(oDefChoice));
    // connect the signal AFTER setCurrentIndex is called
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChangedCB(int)));
    return qobject_cast<QWidget*>(comboBox);
}

void PrinterOptions::currentIndexChangedCB(int index)
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());
    bool isDifferent = comboBox->property("defaultChoice").toInt() != index;
    kDebug() << isDifferent << comboBox->property("different").toBool();
    if (isDifferent != comboBox->property("different").toBool()) {
        isDifferent ? m_changes++ : m_changes--;
        comboBox->setProperty("different", isDifferent);
        emit changed(m_changes);
    }
}

PrinterOptions::~PrinterOptions()
{
    if (m_ppd != NULL) {
        ppdClose(m_ppd);
    }
}

void PrinterOptions::save()
{
    // copy cups-1.4.2/cgi-bin line 3779
//     if (m_changes) {
//         QHash<QString, QVariant> values;
//         if (nameLE->property("different").toBool()) {
//             values["printer-info"] = nameLE->text();
//         }
//         if (locationLE->property("different").toBool()) {
//             values["printer-location"] = locationLE->text();
//         }
//         if (connectionLE->property("different").toBool()) {
//             values["device-uri"] = connectionLE->text();
//         }
//         m_printer->save(values);
//     }
}

bool PrinterOptions::hasChanges()
{
    return m_changes;
}

#include "PrinterOptions.moc"
