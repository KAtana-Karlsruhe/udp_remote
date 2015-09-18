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
