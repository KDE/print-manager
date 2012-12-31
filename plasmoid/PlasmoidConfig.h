#ifndef PLASMOIDCONFIG_H
#define PLASMOIDCONFIG_H

#include <QWidget>

namespace Ui {
class PlasmoidConfig;
}

class PlasmoidConfig : public QWidget
{
    Q_OBJECT
    
public:
    explicit PlasmoidConfig(QWidget *parent = 0);
    ~PlasmoidConfig();
    
private:
    Ui::PlasmoidConfig *ui;
};

#endif // PLASMOIDCONFIG_H
