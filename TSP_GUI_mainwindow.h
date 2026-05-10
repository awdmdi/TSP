#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QVector>
#include <QString>
#include <QPointF>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QSlider>
#include <QMouseEvent>
#include <QEvent>
#include <queue>
#include <vector>
#include <algorithm>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct CityNode {
    QString name;
    QPointF pos;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onAddCity();
    void onSetEdge();
    void onRunBrute();
    void onRunGreedy();
    void onRunDijkstra();
    void onCompare();
    void onClearAll();
    void onCanvasClicked(QPointF pos);

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsView  *graphView;
    QTextEdit      *outputEdit;

    // Input fields
    QLineEdit *cityNameEdit;
    QLineEdit *fromEdit, *toEdit, *weightEdit;
    QLineEdit *startEdit;
    QLineEdit *srcEdit, *dstEdit;
    QSlider   *speedSlider;

    // State
    bool    placingCity = false;
    QString pendingCityName;

    // Graph data
    int numCities = 0;
    QVector<CityNode> cities;
    int adjMatrix[20][20] = {};

    // Drawing items
    QVector<QGraphicsEllipseItem*> cityCircles;
    QVector<QGraphicsTextItem*>    cityLabels;
    QVector<QGraphicsLineItem*>    edgeLines;
    QVector<QGraphicsTextItem*>    edgeWeightLabels;
    QVector<QGraphicsItem*>        highlightItems;

    // Algorithms
    int          routeCost(const QVector<int>& route);
    QVector<int> bruteForceTSP(int start);
    QVector<int> greedyTSP(int start);
    QVector<int> dijkstra(int src, int dst, int &totalCost);

    // Drawing helpers
    void redrawGraph();
    void highlightPath(const QVector<int>& path, bool loop, QColor color);
    void clearHighlights();
    void log(const QString& msg);
};

#endif // MAINWINDOW_H
