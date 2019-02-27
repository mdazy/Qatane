#pragma once

#include <QtCore/QObject>

class QTcpSocket;

class Messenger : public QObject {
    Q_OBJECT;
public:
    Messenger();
    virtual ~Messenger();

private slots:
    void initializeConnection();
    void receiveText();
    void disconnect();

signals:
    void gameCommand( const QString& cmd );
    void chatMessage( const QString& text );

private:
    void send( const QString& message ) const;

private:
    QTcpSocket* socket_;
};