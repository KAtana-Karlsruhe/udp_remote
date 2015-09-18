// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
/*
Copyright (c) 2015, KAtana-Karlsruhe
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Christoph Rist <rist.christoph@gmail.com>
 * \date    2014-10-17
 *
 */
//----------------------------------------------------------------------

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_connected(false)
{
  m_controls.resize(4, false);

  ui->setupUi(this);

  ui->spinBox_speed->setMaximum(MAX_SPEED);
  ui->spinBox_speed->setMinimum(-MAX_BACKWARD);
  ui->spinBox_steer->setMaximum(MAX_RIGHT);
  ui->spinBox_steer->setMinimum(MIN_LEFT);

  // Timer
  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(slotUpdateControl()));

  // Buttons
  connect(ui->pushButton_start, SIGNAL(clicked()), this, SLOT(processButtonClick()));
  connect(ui->pushButton_ende, SIGNAL(clicked()), this, SLOT(processButtonClick()));
  connect(ui->pushButton_trigger, SIGNAL(clicked()), this, SLOT(processButtonClick()));
  connect(ui->pushButton_changeStrip, SIGNAL(clicked()), this, SLOT(processButtonClick()));
  connect(ui->pushButton_junction_left, SIGNAL(clicked()), this, SLOT(processButtonClick()));
  connect(ui->pushButton_junction_right, SIGNAL(clicked()), this, SLOT(processButtonClick()));
  connect(ui->pushButton_junction_straight, SIGNAL(clicked()), this, SLOT(processButtonClick()));

  // Cruise control checkbox
  connect(ui->checkBox_tempomat, SIGNAL(toggled(bool)), this, SLOT(enableCruiseControl(bool)));

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
  else if (btn == ui->pushButton_trigger)
  {
    sendTrigger(ui->spinBox_trigger->value());
  }
  else if (btn == ui->pushButton_changeStrip)
  {
   sendTrigger(6);
  }
  else if (btn == ui->pushButton_junction_right)
  {
   sendTrigger(7);
  }
  else if (btn == ui->pushButton_junction_straight)
  {
   sendTrigger(8);
  }
  else if (btn == ui->pushButton_junction_left)
  {
   sendTrigger(9);
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
        ui->checkBox_tempomat->setCheckState(Qt::Unchecked);
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
  float speed;
  qint8 steering;

  if (ui->checkBox_tempomat->checkState() == Qt::Unchecked)
  {
    speed = m_speed;
    steering = m_steering;

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
  }
  else
  {
    speed = ui->spinBox_speed->value();
    steering = ui->spinBox_steer->value();
  }

  // Check for changes and send
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
void MainWindow::enabledCruiseControl(bool checked)
{
  if (checked)
  {
    sendControl(ui->spinBox_speed->value(), ui->spinBox_steer->value());
  }
  else
  {
    sendControl(0, 0);
  }
}
void MainWindow::sendTrigger(quint8 value)
{
  QByteArray datagram;
  datagram.resize(2);
  datagram[0] = CMD_TRIGGER;
  datagram[1] = value;
  m_socket->write(datagram.data(), datagram.size());
}
