#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QKeyEvent>

const u_int16_t MILLISECONDS = 20;

const u_int8_t FORWARD = 0;
const u_int8_t BACKWARD = 1;
const u_int8_t LEFT = 2;
const u_int8_t RIGHT = 3;

const u_int8_t CMD_CONTROL = 0x5e;
const u_int8_t CMD_ALIVE = 0x3f;
const u_int8_t CMD_TRIGGER = 0x4e;


const int8_t MAX_SPEED = 40;
const int8_t MAX_BACKWARD = 35;
const int8_t MIN_LEFT = -100;
const int8_t MAX_RIGHT = +100;

const int8_t SPEED_STEERING = 5;

const int8_t ACCEL = 1;
const int8_t DECEL = 4;
const float ROLL = 0.1;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow *ui;


  void initSocket(const QHostAddress &address, quint16 port);
  void closeSocket()      { m_socket->close(); }
  void toggleConnected(bool connected);
  void readPendingDatagrams();

  QUdpSocket *m_socket;
  bool m_connected;

  QTimer *m_timer;

  std::vector<bool> m_controls;
  qint8 m_speed;
  qint8 m_steering;

  bool m_lifted;

  void sendControl(qint8 speed, qint8 steering);
  void sendAlive();
  void sendTrigger(quint8 value);

private slots:
  void processButtonClick();
  void keyPressEvent(QKeyEvent* event);
  void keyReleaseEvent(QKeyEvent* event);
  void slotUpdateControl();
  void enabledCruiseControl(bool checked);


};

#endif // MAINWINDOW_H
