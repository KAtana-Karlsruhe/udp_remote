#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_connected(false)
{
  ui->setupUi(this);

  // Timer
  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(slotUpdateControl()));

  // Buttons
  connect(ui->pushButton_start, SIGNAL(clicked()), this, SLOT(processButtonClick()));
  connect(ui->pushButton_ende, SIGNAL(clicked()), this, SLOT(processButtonClick()));

  // Keyboard focus
  setFocusPolicy(Qt::StrongFocus);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::initSocket(const QHostAddress& address, quint16 port)
{
  m_socket = new QUdpSocket(this);
  m_socket->connectToHost(address, port);
}

void MainWindow::processButtonClick()
{
  QPushButton* btn = dynamic_cast<QPushButton*>(sender());

  if (btn == ui->pushButton_start)
  {
    toggleConnected(true);

    QString addr = ui->lineEdit_ip->text();
    QHostAddress address(addr);
    //QHostAddress address = QHostAddress::LocalHost;
    qint16 port = ui->spinBox_port->value();

    initSocket(address, port);

    m_controls.resize(4, false);
    m_steering = 0;
    m_speed = 0;
    m_lifted = false;

    m_timer->start(MILLISECONDS);
  }
  else if (btn == ui->pushButton_ende)
  {
    m_timer->stop();

    toggleConnected(false);
    closeSocket();
  }
}

void MainWindow::toggleConnected(bool connected)
{
  m_connected = connected;
  ui->pushButton_start->setEnabled(!connected);
  ui->pushButton_ende->setEnabled(connected);
  ui->spinBox_port->setEnabled(!connected);
  ui->lineEdit_ip->setEnabled(!connected);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
  if (event->isAutoRepeat())
  {
    event->ignore();
    return;
  }
  else if (event->type() == QEvent::KeyPress)
  {
    switch(event->key())
    {
      case Qt::Key_Escape:
        m_speed = 0;
        sendControl(0, m_steering);
        return;
      case Qt::Key_Up:
        m_controls[FORWARD] = true;
        break;
      case Qt::Key_Down:
        m_controls[BACKWARD] = true;
        break;
      case Qt::Key_Left:
        m_controls[LEFT] = true;
        break;
      case Qt::Key_Right:
        m_controls[RIGHT] = true;
        break;
      default:
        event->ignore();
    }
    return;
  }

  event->ignore();
}
void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
  if (event->isAutoRepeat())
  {
    event->ignore();
    return;
  }
  else if (event->type() == QEvent::KeyRelease)
  {
    switch(event->key())
    {
      case Qt::Key_Up:
        m_controls[FORWARD] = false;
        break;
      case Qt::Key_Down:
        m_controls[BACKWARD] = false;
        break;
      case Qt::Key_Left:
        m_controls[LEFT] = false;
        break;
      case Qt::Key_Right:
        m_controls[RIGHT] = false;
        break;
      default:
        event->ignore();
    }
    return;
  }

  event->ignore();
}
void MainWindow::slotUpdateControl()
{
  float speed = m_speed;
  qint8 steering = m_steering;

  if (m_controls[BACKWARD])
  {
    if (speed > DECEL)
      speed -= DECEL;
    else if (speed <= 0)
    {
      if (speed > (-MAX_BACKWARD + ACCEL) && m_lifted)
      {
        speed -= ACCEL;
      }
    }
    else
      speed = 0;
  }
  else if (m_controls[FORWARD])
  {
    m_lifted = false;
    if (speed <= (MAX_SPEED-ACCEL))
      speed += ACCEL;
  }
  else
  {
    if (speed > ROLL)
      speed -= ROLL;
    else if (speed < -ROLL)
      speed += ROLL;
    else
    {
      m_lifted = true;
      speed = 0;
    }
  }
  if (m_controls[LEFT])
  {
    if (steering >= (MIN_LEFT + SPEED_STEERING))
      steering -= SPEED_STEERING;
  }
  else if (m_controls[RIGHT])
  {
    if (steering <= (MAX_RIGHT - SPEED_STEERING))
      steering += SPEED_STEERING;
  }
  else
  {
    if (steering > 0)
    {
      if (steering > SPEED_STEERING)
        steering -= SPEED_STEERING;
      else
        steering = 0;
    }
    else if (steering < 0)
    {
      if (steering < -SPEED_STEERING)
        steering += SPEED_STEERING;
      else
        steering = 0;
    }
  }

  if (speed != m_speed || steering != m_steering)
  {
    m_speed = (qint8)speed;
    m_steering = steering;
    sendControl((qint8)speed, steering);
  }
  else
    sendAlive();
}

void MainWindow::sendControl(qint8 speed, qint8 steering)
{
  std::cout <<"send " <<(int16_t)speed <<" " <<(int16_t)steering <<std::endl;
  QByteArray datagram;
  datagram.resize(3);
  datagram[0] = CMD_CONTROL;
  datagram[1] = speed;
  datagram[2] = steering;
  m_socket->write(datagram.data(), datagram.size());
}

void MainWindow::sendAlive()
{
  QByteArray datagram;
  datagram.resize(1);
  datagram[0] = CMD_ALIVE;
  m_socket->write(datagram.data(), datagram.size());
}

