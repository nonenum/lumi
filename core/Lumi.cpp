// Copyright (c) nonenum 2026.

#include "Lumi.h"

Lumi::Lumi(QWidget *parent) : QWidget(parent), m_physics_state(PhysicsState::FALLING), m_action_state(ActionState::IDLE), m_velocity_y(0.0) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    LoadSprites();
    SetupSystemTray();

    resize(G_DISPLAY_WIDTH, G_DISPLAY_HEIGHT);

    SetAction(ActionState::IDLE);
    UpdateSprite();

    m_physics_timer = new QTimer(this);
    connect(m_physics_timer, &QTimer::timeout, this, &Lumi::UpdatePhysics);
    m_physics_timer->start(16);

    m_decision_timer = new QTimer(this);
    connect(m_decision_timer, &QTimer::timeout, this, &Lumi::Decide);
    m_decision_timer->start(4000);

    CalcFloor();

    int spawn_x = m_primary_display_area.x() + 100;
    int spawn_y = m_primary_display_area.y() + 100;
    move(spawn_x, spawn_y);
}

Lumi::~Lumi() {}

void Lumi::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (!m_sprite.isNull()) {
        double pivot_x = width() / 2.0;
        double pivot_y = 20.0;

        painter.translate(pivot_x, pivot_y);
        painter.rotate(m_current_angle);
        painter.translate(-pivot_x, -pivot_y);

        painter.drawPixmap((G_DISPLAY_WIDTH / 2) - (G_SPRITE_WIDTH / 2), 0, m_sprite);
    } else {
        painter.setBrush(Qt::magenta);
        painter.drawRect(rect());
    }
}

void Lumi::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_physics_state = PhysicsState::GRABBED;

        SetAction(ActionState::IDLE);

        m_drag_offset = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_last_mouse_x = x();
        m_last_mouse_y = y();
        m_velocity_x = 0.0;
        m_velocity_y = 0.0;
    }
}

void Lumi::mouseMoveEvent(QMouseEvent *event) {
    if (m_physics_state == PhysicsState::GRABBED && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_drag_offset);
    }
}

void Lumi::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        CalcFloor();

        if (std::abs(m_velocity_x) > 2.0 || std::abs(m_velocity_y) > 2.0) {
            m_physics_state = PhysicsState::THROWN;

            if (m_velocity_x > MAX_VELOCITY) m_velocity_x = MAX_VELOCITY;
            if (m_velocity_x < -MAX_VELOCITY) m_velocity_x = -MAX_VELOCITY;

            if (m_velocity_y > MAX_VELOCITY) m_velocity_y = MAX_VELOCITY;
            if (m_velocity_y < -MAX_VELOCITY) m_velocity_y = -MAX_VELOCITY;
        } else {
            m_physics_state = PhysicsState::FALLING;
            m_velocity_y = 0.0;
        }
    }
}

void Lumi::contextMenuEvent(QContextMenuEvent *event) {
    QMenu context_menu(this);
    QAction* quit = context_menu.addAction("Bye Lumi!");

    connect(quit, &QAction::triggered, qApp, &QApplication::quit);

    context_menu.exec(event->globalPos());
}

void Lumi::UpdatePhysics() {
    double target_angle = 0.0;

    if (m_physics_state == PhysicsState::GRABBED) {
        m_sprite = m_asset_grabbed;

        m_velocity_x = x() - m_last_mouse_x;
        m_velocity_y = y() - m_last_mouse_y;

        m_last_mouse_x = x();
        m_last_mouse_y = y();

        target_angle = m_velocity_x * 1.5;

        if (target_angle > 45.0) target_angle = 45.0;
        if (target_angle < -45.0) target_angle = -45.0;
    } else if (m_physics_state == PhysicsState::THROWN || m_physics_state == PhysicsState::FALLING) {
        m_sprite = m_asset_falling;

        m_velocity_x *= AIR_RESISTANCE;
        m_velocity_y += GRAVITY;

        if (m_velocity_y > MAX_FALL_SPEED) m_velocity_y = MAX_FALL_SPEED;

        int new_x = x() + static_cast<int>(m_velocity_x);
        int new_y = y() + static_cast<int>(m_velocity_y);

        if (new_x <= m_primary_display_area.left()) {
            new_x = m_primary_display_area.left();
            m_velocity_x *= BOUNCE_DAMPING;
        } else if (new_x + width() >= m_primary_display_area.right()) {
            new_x = m_primary_display_area.right() - width();
            m_velocity_x *= BOUNCE_DAMPING;
        }

        if (new_y >= m_floor_y) {
            new_y = m_floor_y;

            if (m_physics_state == PhysicsState::THROWN && m_velocity_y > 5.0) {
                m_velocity_y *= BOUNCE_DAMPING;

                if (std::abs(m_velocity_y) < 2.0) {
                    m_sprite = m_assets[ActionState::IDLE];
                    m_physics_state = PhysicsState::STABLE;

                    m_velocity_x = 0.0;
                    m_velocity_y = 0.0;
                }
            } else {
                m_sprite = m_assets[ActionState::IDLE];
                m_physics_state = PhysicsState::STABLE;

                m_velocity_x = 0.0;
                m_velocity_y = 0.0;
            }
        }

        move(new_x, new_y);
        target_angle = 0.0;
    } else if (m_physics_state == PhysicsState::STABLE) {
        if (m_action_state == ActionState::WALKING) {
            int momentary_goal = m_walking_direction;

            CheckWalkingState();
            if (m_walking_direction != momentary_goal) UpdateSprite();

            move(x() + (2 * m_walking_direction), y());
            target_angle = 0.0;
        }
    }

    m_current_angle += (target_angle - m_current_angle) * 0.2;
    update();
}

void Lumi::Decide() {
    if (m_physics_state != PhysicsState::STABLE) return;

    int choice = QRandomGenerator::global()->bounded(4);

    switch (choice) {
        case 0: SetAction(ActionState::IDLE); break;
        case 1: SetAction(ActionState::WALKING); break;
        case 2: SetAction(ActionState::SCROLLING_PHONE); break;
        case 3: SetAction(ActionState::SLEEPING); break;
    }
}

void Lumi::SetAction(ActionState a_new_state) {
    if (m_action_state == a_new_state) return;
    m_action_state = a_new_state;

    UpdateSprite();
}

void Lumi::CalcFloor() {
    QScreen* screen = QGuiApplication::primaryScreen();

    if (screen) {
        m_primary_display_area = screen->availableGeometry();

        m_floor_y = m_primary_display_area.bottom() - height();
    }
}

void Lumi::CheckWalkingState() {
    int boundary_offset = 250;

    if (x() > m_primary_display_area.width() - boundary_offset) {
        m_walking_direction = -1;
    } else if (x() < boundary_offset) {
        m_walking_direction = 1;
    }
}

void Lumi::UpdateSprite() {
    if (!m_assets.contains(m_action_state)) return;

    QPixmap frame = m_assets[m_action_state];

    if (m_walking_direction == -1) {
        QTransform flip;
        flip.scale(-1, 1);

        frame = frame.transformed(flip);
    }

    m_sprite = frame;
    update();
}

void Lumi::LoadSprites() {
    m_assets[ActionState::IDLE] = QPixmap("../res/idle.png").scaled(G_SPRITE_WIDTH, G_SPRITE_HEIGHT);
    m_assets[ActionState::WALKING] = QPixmap("../res/idle.png").scaled(G_SPRITE_WIDTH, G_SPRITE_HEIGHT);
    m_assets[ActionState::SCROLLING_PHONE] = QPixmap("../res/phone.png").scaled(G_SPRITE_WIDTH, G_SPRITE_HEIGHT);
    m_assets[ActionState::SLEEPING] = QPixmap("../res/sleeping.png").scaled(G_SPRITE_WIDTH, G_SPRITE_HEIGHT);
}

void Lumi::SetupSystemTray() {
    m_tray_icon = new QSystemTrayIcon(this);

    QIcon icon = style()->standardIcon(QStyle::SP_ComputerIcon);

    m_tray_icon->setIcon(icon);
    m_tray_icon->setToolTip("Lumi");

    m_tray_menu = new QMenu(this);

    QAction* quit = m_tray_menu->addAction("Bye Lumi!");

    connect(quit, &QAction::triggered, qApp, &QApplication::quit);

    m_tray_icon->setContextMenu(m_tray_menu);
    m_tray_icon->show();
}
