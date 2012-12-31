#include "PlasmoidConfig.h"
#include "ui_PlasmoidConfig.h"

PlasmoidConfig::PlasmoidConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlasmoidConfig)
{
    ui->setupUi(this);
}

PlasmoidConfig::~PlasmoidConfig()
{
    delete ui;
}
