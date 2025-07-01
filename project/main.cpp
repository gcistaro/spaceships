#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QTimer>
#include <QPixmap>
#include <QSoundEffect>
#include <QVector>
#include <cstdlib>
#include <ctime>

enum GameState { Menu, Playing, GameOver };

struct Bullet {
    int x, y;
};

struct Enemy {
    int x, y;
};

const int WIDTH = 400;
const int HEIGHT = 600;
const int SHIP_WIDTH = 40;
const int SHIP_HEIGHT = 20;
const int BULLET_WIDTH = 5;
const int BULLET_HEIGHT = 10;
const int ENEMY_WIDTH = 30;
const int ENEMY_HEIGHT = 20;

class GameWidget : public QWidget {
    Q_OBJECT

public:
    GameWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(WIDTH, HEIGHT);
        state = Menu;
        shipX = WIDTH / 2 - SHIP_WIDTH / 2;
        shipY = HEIGHT - SHIP_HEIGHT - 10;
        score = 0;

        shipImg.load(":/images/ship.png");
        enemyImg.load(":/images/enemy.png");
        bulletImg.load(":/images/bullet.png");
        background.load(":/images/background.png");

        shootSound.setSource(QUrl("qrc:/sounds/shoot.wav"));
        shootSound.setVolume(0.25f);

        qsrand(static_cast<uint>(time(nullptr)));

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &GameWidget::gameLoop);
        timer->start(30);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);

        // Background
        painter.drawPixmap(0, 0, WIDTH, HEIGHT, background);

        if (state == Menu) {
            drawCenteredText(painter, "Press SPACE to Start", Qt::white, HEIGHT / 2);
            return;
        } else if (state == GameOver) {
            drawCenteredText(painter, "Game Over\nPress R to Restart", Qt::red, HEIGHT / 2);
            return;
        }

        // Draw spaceship
        painter.drawPixmap(shipX, shipY, SHIP_WIDTH, SHIP_HEIGHT, shipImg);

        // Draw bullets
        for (const Bullet &b : bullets)
            painter.drawPixmap(b.x, b.y, BULLET_WIDTH, BULLET_HEIGHT, bulletImg);

        // Draw enemies
        for (const Enemy &e : enemies)
            painter.drawPixmap(e.x, e.y, ENEMY_WIDTH, ENEMY_HEIGHT, enemyImg);

        // Draw score
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 12));
        painter.drawText(10, 20, "Score: " + QString::number(score));
    }

    void keyPressEvent(QKeyEvent *event) override {
        if (state == Menu && event->key() == Qt::Key_Space) {
            startGame();
            return;
        }

        if (state == GameOver && event->key() == Qt::Key_R) {
            startGame();
            return;
        }

        if (state != Playing) return;

        if (event->key() == Qt::Key_Left && shipX > 0)
            shipX -= 10;
        else if (event->key() == Qt::Key_Right && shipX + SHIP_WIDTH < WIDTH)
            shipX += 10;
        else if (event->key() == Qt::Key_Space) {
            bullets.append({shipX + SHIP_WIDTH / 2 - 2, shipY});
            shootSound.play();
        }
    }

private slots:
    void gameLoop() {
        if (state != Playing) return;

        // Move bullets
        for (Bullet &b : bullets) b.y -= 10;
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                                     [](Bullet &b) { return b.y < 0; }),
                      bullets.end());

        // Spawn enemies
        if (qrand() % 20 == 0) {
            int x = qrand() % (WIDTH - ENEMY_WIDTH);
            enemies.append({x, 0});
        }

        // Move enemies
        for (Enemy &e : enemies) e.y += 5;

        // Collision detection
        for (int i = 0; i < bullets.size(); ++i) {
            for (int j = 0; j < enemies.size(); ++j) {
                if (bullets[i].x < enemies[j].x + ENEMY_WIDTH &&
                    bullets[i].x + BULLET_WIDTH > enemies[j].x &&
                    bullets[i].y < enemies[j].y + ENEMY_HEIGHT &&
                    bullets[i].y + BULLET_HEIGHT > enemies[j].y) {
                    bullets.remove(i);
                    enemies.remove(j);
                    score += 10;
                    --i;
                    break;
                }
            }
        }

        // Game Over if enemies reach ship
        for (const Enemy &e : enemies) {
            if (e.y + ENEMY_HEIGHT >= shipY) {
                state = GameOver;
                timer->stop();
                break;
            }
        }

        // Clean up enemies
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                                     [](Enemy &e) { return e.y > HEIGHT; }),
                      enemies.end());

        update();
    }

private:
    void startGame() {
        state = Playing;
        bullets.clear();
        enemies.clear();
        score = 0;
        shipX = WIDTH / 2 - SHIP_WIDTH / 2;
        timer->start(30);
    }

    void drawCenteredText(QPainter &p, const QString &text, QColor color, int y) {
        p.setPen(color);
        p.setFont(QFont("Arial", 16, QFont::Bold));
        QRect rect(0, y - 20, WIDTH, 100);
        p.drawText(rect, Qt::AlignHCenter | Qt::AlignTop, text);
    }

    GameState state;
    int shipX, shipY;
    int score;

    QVector<Bullet> bullets;
    QVector<Enemy> enemies;
    QTimer *timer;

    QPixmap shipImg, enemyImg, bulletImg, background;
    QSoundEffect shootSound;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    GameWidget game;
    game.setWindowTitle("Qt Spaceship Game");
    game.show();
    return app.exec();
}

#include "main.moc"
