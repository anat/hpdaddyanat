#include "netsoul.h"
#include "ui_netsoul.h"
#include <string>
#include <cstdio>
#include "packet.h"
Netsoul::Netsoul(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::Netsoul),
	portA(0),
	portB(0)
{
    ui->setupUi(this);
}

Netsoul::~Netsoul()
{
    delete ui;
}


bool Netsoul::isProtocol(Packet & p)
{

    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    const char * begin[] = {"salut", "auth_ag", "list_users", "ping", "user_cmd", "state", "exit", NULL};

    tcp* pTCP = (tcp*)p.getBuffer();
    bool isProtocol = false;
    if (portA && portB)
    {
        if ((pTCP->source == portA && pTCP->dest == portB) ||
            (pTCP->source == portB && pTCP->dest == portA))
            isProtocol = true;
    }
    else
    {
        int i = 0;
	while (begin[i])
        {
            if (!strncmp(begin[i], data, strlen(begin[i])))
	    {
                portA = pTCP->source;
                portB = pTCP->dest;
                isProtocol = true;
	    }
            i++;
        }
    }
    return isProtocol;
}

void Netsoul::addActivity(QString & message)
{
    this->ui->activity->setPlainText("\n- " + message + this->ui->activity->toPlainText());
}




int Netsoul::sendTargetAToTargetB(Packet & p)
{
    std::string * msg;
    tcp* pTCP = static_cast<tcp*>(p.getBuffer());

    QString message("A>>> size = " + QString::number(p.Size - sizeof(tcp))
                    + "\tisACK = " + QString::number(pTCP->ack) + "\tseq = " + QString::number(pTCP->seq) + "\tack = " + QString::number(pTCP->ack_seq));
    //this->addActivity(message);



    if ((msg = this->isMessage(p)))
    {
        QString message("A>>> Got a ns message (" + QString(msg->c_str()) + ")");
        this->addActivity(message);
        //std::cout << "Got a ns message (\"" << msg << "\")\n"<< std::endl;


/*
        char * data = ((char*)p.getBuffer()) + sizeof(tcp);
        char buffer[p.Size - sizeof(tcp)];
        memcpy(buffer, data, p.Size - sizeof(tcp));
        buffer[p.Size - sizeof(tcp) - 1] = 0;
        QString str("A>>> Unrecognized Packet : \"");
        str += (const char *)buffer;
        str += "\"";
        this->addActivity(str);
        */
    }
    return 0;
}


int Netsoul::sendTargetBToTargetA(Packet & p)
{
    std::string * msg;
    tcp* pTCP = static_cast<tcp*>(p.getBuffer());

    QString message("<<<B size = " + QString::number(p.Size - sizeof(tcp))
                    + "\tisACK = " + QString::number(pTCP->ack) + "\tseq = " + QString::number(pTCP->seq) + "\tack = " + QString::number(pTCP->ack_seq));
    //this->addActivity(message);



    if ((msg = this->isMessage(p)))
    {
        QString message("<<<B Got a ns message (" + QString(msg->c_str()) + ")");
        this->addActivity(message);
        //std::cout << "Got a ns message (\"" << msg << "\")"<< std::endl;

        /*
        char * data = ((char*)p.getBuffer()) + sizeof(tcp);
        char buffer[p.Size - sizeof(tcp)];
        memcpy(buffer, data, p.Size - sizeof(tcp));
        buffer[p.Size - sizeof(tcp) - 1] = 0;
        QString str("<<<B Unrecognized Packet : \"");
        str += (const char *)buffer;
        str += "\"";
        this->addActivity(str);
        */
    }
    return 0;
}

std::string *Netsoul::isMessage(Packet & p)
{
    char *data = ((char*)p.getBuffer()) + sizeof(tcp);
    int i;
    std::string *str = new std::string();

    if (!strncmp(NS_SENDMSG, data, strlen(NS_SENDMSG)))
    {
	data += strlen(NS_SENDMSG);
	for (i = 0; data[i]; i++)
        {
            if (!strncmp("msg ", data+i, 4) && i)
            {
                if ((i < 9 || strncmp(NS_SENDMSG, data+i-9, 8)))
                {
                    //return new std::string(data+i+4);
                    int end = 0, start = 0;
                    if (data[i+4] != '_')
                        start = 0;
                    else
                    {
                        start += 3;
                        while (data[i+4+start] != '_') start++;
                        start++;
                    }
                    while (data[i + 4 + end] != ' ') end++;
                    char buffer[end - start + 1];
                    memcpy(buffer, data + i + 4 + start, end - start);
                    buffer[end - start] = 0;
                    *str += (const char *)buffer;
                    return str;

                }
            }
        }
    }
    return NULL;
}