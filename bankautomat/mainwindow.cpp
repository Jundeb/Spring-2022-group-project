#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
     ui->setupUi(this);

    objectDLLSerialPort = new DLLSerialPort;
    objectDLLPinCode = new DLLPinCode;
    objectDLLRESTAPI = new DLLRESTAPI;

    objectpaakayttoliittyma = new paakayttoliittyma;
    objectCreditOrDebit = new creditOrDebit;
    objectNostaRahaa = new NostaRahaa;
    objectTalletaRahaa = new TalletaRahaa;

    connect(objectDLLSerialPort, SIGNAL(kortinNumeroSignal(QString)),
            this, SLOT(kortinNumeroSlot(QString)));

    connect(objectDLLPinCode, SIGNAL(pinkoodiInterface(QString)),
           this, SLOT(pinkoodiSlot(QString)));

    connect(objectDLLRESTAPI, SIGNAL(loginSignalToExe(QString)),
            this, SLOT(loginSlot(QString)));

    connect(objectDLLRESTAPI, SIGNAL(tiedotListToExe(QStringList)),
            this, SLOT(asiakasTiedotSlot(QStringList)));

    connect(objectDLLRESTAPI, SIGNAL(tilitapahtumatToExe(QStringList)),
            this, SLOT(tilitapahtumatSlot(QStringList)));

    connect(objectCreditOrDebit, SIGNAL(tilinValinta(QString)),
            this, SLOT(tiliValittuSlot(QString)));

    connect(objectpaakayttoliittyma, SIGNAL(nostaRahaaValittu()),
            this, SLOT(nostaRahaaValittuSlot()));

    connect(objectNostaRahaa, SIGNAL(nostaRahaa(QString)),
            this, SLOT(nostaRahaaSlot(QString)));

    connect(objectpaakayttoliittyma, SIGNAL(talletaRahaaValittu()),
            this, SLOT(talletaRahaaValittuSlot()));

    connect(objectTalletaRahaa, SIGNAL(talletaRahaa(QString)),
            this, SLOT(talletaRahaaSlot(QString)));

    connect(objectCreditOrDebit, SIGNAL(kirjauduUlosSignal()),
            this, SLOT(kirjauduUlosSlot()));

    connect(objectpaakayttoliittyma, SIGNAL(tilitapahtumaValinta(QString)),
            this, SLOT(tilitapahtumaValintaSlot(QString)));

    connect(objectpaakayttoliittyma, SIGNAL(kirjauduUlosSignal()),
            this, SLOT(kirjauduUlosSlot()));

    connect(objectNostaRahaa, SIGNAL(PaaTimerNSignal()),
            this, SLOT(PaaTimerSlot()));

    connect(objectTalletaRahaa, SIGNAL(PaaTimerTSignal()),
            this, SLOT(PaaTimerSlot()));


}

MainWindow::~MainWindow()
{
    delete ui;
    delete objectDLLSerialPort;
    delete objectDLLPinCode;
    delete objectDLLRESTAPI;

    ui = nullptr;
    objectDLLSerialPort = nullptr;
    objectDLLPinCode = nullptr;
    objectDLLRESTAPI = nullptr;
}


void MainWindow::on_pushButton_clicked()
{
   objectDLLPinCode->naytaPinkoodiKayttoliittyma();
}

void MainWindow::kortinNumeroSlot(QString kortinnumeroDLL)
{
    qDebug() << "Kortinnumero exessä: " << kortinnumeroDLL;
    kortinnumero = kortinnumeroDLL;
    objectDLLPinCode->naytaPinkoodiKayttoliittyma();
}

void MainWindow::pinkoodiSlot(QString pinkoodiDLL)
{
    qDebug() << "syötetty pinkoodi exessä: " << pinkoodiDLL;
    objectDLLRESTAPI->login("0600064972", pinkoodiDLL);
}

void MainWindow::loginSlot(QString login)
{
    qDebug() << "login: " << login;

    if(login == "true")
      {       
        objectDLLRESTAPI->haeAsiakkaanTiedot("0600064972");
      }
    else if(login == "false")
      {
        objectDLLPinCode->pinkoodiVaarin();
      }
}

void MainWindow::asiakasTiedotSlot(QStringList tiedotLista)
{
    id_Tili = tiedotLista[0];
    id_Asiakas= tiedotLista[1];
    nimi = tiedotLista[2];
    debitTilinumero = tiedotLista[3];
    creditTilinumero = tiedotLista[4];
    debitSaldo = tiedotLista[5];
    creditSaldo = tiedotLista[6];
    luottoraja = tiedotLista[7];

    objectDLLRESTAPI->haeTilitapahtumat(id_Tili, "5", "0");

    if(creditTilinumero == NULL)
    {
        valinta = "debit";
        objectpaakayttoliittyma->objectPaaTimer->start(30000);
        objectpaakayttoliittyma->show();
    }
    else if(creditTilinumero != NULL)
    {
        objectCreditOrDebit->objectTimerCredDeb->start(10000);
        objectCreditOrDebit->show();
    }
}

void MainWindow::tilitapahtumatSlot(QStringList paramTilitapahtumat)
{
    tapahtumatListaPituus = paramTilitapahtumat.count();
    tilitapahtumat = paramTilitapahtumat;
    
    if(valinta == "debit")
    {
         objectpaakayttoliittyma->asetaTiedot(valinta, nimi, debitSaldo, NULL, tilitapahtumat);
    }
    else if(valinta=="credit")
    {
        objectpaakayttoliittyma->asetaTiedot(valinta, nimi, creditSaldo, luottoraja, tilitapahtumat);
    }

}

void MainWindow::tiliValittuSlot(QString tilinValinta)
{
    objectCreditOrDebit->close();
    valinta = tilinValinta;

    if(tilinValinta == "debit")
    {
        objectpaakayttoliittyma->asetaTiedot(valinta, nimi, debitSaldo, NULL, tilitapahtumat);
    }
    else if(tilinValinta == "credit")
    {
        objectpaakayttoliittyma->asetaTiedot(valinta, nimi, creditSaldo, luottoraja, tilitapahtumat);
    }
    objectpaakayttoliittyma->objectPaaTimer->start(30000);
    objectpaakayttoliittyma->show();
}

void MainWindow::nostaRahaaValittuSlot()
{
    objectNostaRahaa->objectTimerNosta->start(10000);
    objectNostaRahaa->show();
}

void MainWindow::nostaRahaaSlot(QString nostoSumma)
{
    if(valinta == "debit" && nostoSumma.toFloat() <= debitSaldo.toFloat())
    {       
        objectDLLRESTAPI->suoritaDebitNosto(id_Tili, debitTilinumero, "0600064972", debitSaldo, nostoSumma);

        debitSaldo = QString::number(debitSaldo.toFloat() - nostoSumma.toFloat());
    }
    else if(valinta == "credit" && creditSaldo.toFloat() - nostoSumma.toFloat() >= -(luottoraja.toFloat()))
    {    
        objectDLLRESTAPI->suoritaCreditNosto(id_Tili, creditTilinumero, "0600064972", creditSaldo, nostoSumma, luottoraja);

        creditSaldo = QString::number(creditSaldo.toFloat() - nostoSumma.toFloat());
    }
    else
     {
        objectNostaRahaa->virheIlmoitus();
     }

    QTimer::singleShot(500, this, SLOT(singleShotTilitapahtumaSlot()));
}

void MainWindow::talletaRahaaValittuSlot()
{
    objectTalletaRahaa->objectTimerTalleta->start(10000);
    objectTalletaRahaa->show();
}


void MainWindow::talletaRahaaSlot(QString talletusSumma)
{
    if(valinta == "debit")
    {
        objectDLLRESTAPI->suoritaTalletus(valinta, id_Tili, debitTilinumero, "0600064972", talletusSumma);

        debitSaldo = QString::number(debitSaldo.toFloat() + talletusSumma.toFloat());

    }
    else if(valinta == "credit")
    {
       objectDLLRESTAPI->suoritaTalletus(valinta, id_Tili, creditTilinumero, "0600064972", talletusSumma);

       creditSaldo = QString::number(creditSaldo.toFloat() + talletusSumma.toFloat());
    }

    QTimer::singleShot(500, this, SLOT(singleShotTilitapahtumaSlot()));
}

void MainWindow::tilitapahtumaValintaSlot(QString valinta)
{
    if(valinta == "ylos" && kiinteaHakuMaara - 10 >= 0)
    {    
        kiinteaHakuMaara = kiinteaHakuMaara - 10;
        objectDLLRESTAPI->haeTilitapahtumat(id_Tili, "10", QString::number(kiinteaHakuMaara));
    }
    else if(valinta == "alas" && (tapahtumatListaPituus == 10 || paikallinenBoolean == true))
    {      
        paikallinenBoolean = false;
        kiinteaHakuMaara = kiinteaHakuMaara + 10;
        objectDLLRESTAPI->haeTilitapahtumat(id_Tili, "10", QString::number(kiinteaHakuMaara));
    }
}

void MainWindow::singleShotTilitapahtumaSlot()
{
    objectDLLRESTAPI->haeTilitapahtumat(id_Tili, "10", "0");
}

void MainWindow::kirjauduUlosSlot()
{
    delete objectDLLSerialPort;
    delete objectDLLPinCode;
    delete objectDLLRESTAPI;

    delete objectpaakayttoliittyma;
    delete objectCreditOrDebit;
    delete objectNostaRahaa;
    delete objectTalletaRahaa;

    objectDLLSerialPort = nullptr;
    objectDLLPinCode = nullptr;
    objectDLLRESTAPI = nullptr;

    objectpaakayttoliittyma = nullptr;
    objectCreditOrDebit = nullptr;
    objectNostaRahaa = nullptr;
    objectTalletaRahaa = nullptr;

    kortinnumero = nullptr;
    id_Tili = nullptr;
    id_Asiakas = nullptr;
    nimi = nullptr;
    debitTilinumero = nullptr;
    creditTilinumero = nullptr;
    debitSaldo = nullptr;
    creditSaldo = nullptr;
    valinta = nullptr;
    luottoraja = nullptr;
    tilitapahtumat.clear();
    kiinteaHakuMaara = 0;
    tapahtumatListaPituus = 0;
    paikallinenBoolean = true;

    objectDLLSerialPort = new DLLSerialPort;
    objectDLLPinCode = new DLLPinCode;
    objectDLLRESTAPI = new DLLRESTAPI;

    objectpaakayttoliittyma = new paakayttoliittyma;
    objectCreditOrDebit = new creditOrDebit;
    objectNostaRahaa = new NostaRahaa;
    objectTalletaRahaa = new TalletaRahaa;

    connect(objectDLLSerialPort, SIGNAL(kortinNumeroSignal(QString)),
            this, SLOT(kortinNumeroSlot(QString)));

    connect(objectDLLPinCode, SIGNAL(pinkoodiInterface(QString)),
           this, SLOT(pinkoodiSlot(QString)));

    connect(objectDLLRESTAPI, SIGNAL(loginSignalToExe(QString)),
            this, SLOT(loginSlot(QString)));

    connect(objectDLLRESTAPI, SIGNAL(tiedotListToExe(QStringList)),
            this, SLOT(asiakasTiedotSlot(QStringList)));

    connect(objectDLLRESTAPI, SIGNAL(tilitapahtumatToExe(QStringList)),
            this, SLOT(tilitapahtumatSlot(QStringList)));

    connect(objectCreditOrDebit, SIGNAL(tilinValinta(QString)),
            this, SLOT(tiliValittuSlot(QString)));

    connect(objectpaakayttoliittyma, SIGNAL(nostaRahaaValittu()),
            this, SLOT(nostaRahaaValittuSlot()));

    connect(objectNostaRahaa, SIGNAL(nostaRahaa(QString)),
            this, SLOT(nostaRahaaSlot(QString)));

    connect(objectpaakayttoliittyma, SIGNAL(talletaRahaaValittu()),
            this, SLOT(talletaRahaaValittuSlot()));

    connect(objectTalletaRahaa, SIGNAL(talletaRahaa(QString)),
            this, SLOT(talletaRahaaSlot(QString)));

    connect(objectCreditOrDebit, SIGNAL(kirjauduUlosSignal()),
            this, SLOT(kirjauduUlosSlot()));

    connect(objectpaakayttoliittyma, SIGNAL(tilitapahtumaValinta(QString)),
            this, SLOT(tilitapahtumaValintaSlot(QString)));

    connect(objectpaakayttoliittyma, SIGNAL(kirjauduUlosSignal()),
            this, SLOT(kirjauduUlosSlot()));

}

void MainWindow::PaaTimerSlot()
{
    objectpaakayttoliittyma->objectPaaTimer->start(30000);
}

