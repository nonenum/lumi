// Copyright (c) nonenum 2026.

#ifndef LUMI_LUMI_H
#define LUMI_LUMI_H

#include <QWidget>
#include <QPixmap>
#include <QTimer>
#include <QPoint>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QRandomGenerator>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>

#include "Globals.h"

enum class PhysicsState {
    STABLE,
    FALLING,
    GRABBED,
    THROWN
};

enum class ActionState {
    IDLE,
    WALKING,
    SCROLLING_PHONE,
    SLEEPING
};

class Lumi : public QWidget {
public:
    explicit Lumi(QWidget* parent = nullptr);
    ~Lumi() override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void UpdatePhysics();
    void Decide();

private:
    PhysicsState m_physics_state;
    ActionState m_action_state;

    QPixmap m_sprite;
    QPixmap m_asset_grabbed = QPixmap("../res/grabbed.png").scaled(G_SPRITE_WIDTH, G_SPRITE_HEIGHT);
    QPixmap m_asset_falling = QPixmap("../res/falling.png").scaled(G_SPRITE_WIDTH, G_SPRITE_HEIGHT);
    QMap<ActionState, QPixmap> m_assets;

    QPoint m_drag_offset;
    QRect m_primary_display_area;

    double m_velocity_x;
    double m_velocity_y;
    double m_current_angle = 0.0;

    int m_floor_y;
    int m_walking_direction = 1;
    int m_last_mouse_x;
    int m_last_mouse_y;
    int m_padding = 50;

    const double GRAVITY = 0.8;
    const double MAX_FALL_SPEED = 15.0;
    const double MAX_VELOCITY = 30;
    const double AIR_RESISTANCE = 0.95;
    const double BOUNCE_DAMPING = -0.7;

    QTimer* m_physics_timer;
    QTimer* m_decision_timer;

    void SetAction(ActionState a_new_state);
    void CalcFloor();
    void CheckWalkingState();
    void UpdateSprite();
    void LoadSprites();
};

#endif