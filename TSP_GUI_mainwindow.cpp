#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsView>
#include <QTimer>
#include <QSlider>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QColor>
#include <QtMath>
#include <algorithm>

// ─────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("TSP Visualizer — Qt");
    setMinimumSize(1000, 680);

    // Dark background on the main window
    setStyleSheet(R"(
        QMainWindow { background: #1e1e2e; }
        QWidget     { background: #1e1e2e; color: #cdd6f4;
                      font-family: 'Segoe UI'; font-size: 13px; }
        QPushButton { background: #313244; color: #cdd6f4; border: none;
                      border-radius: 6px; padding: 6px 14px; }
        QPushButton:hover   { background: #45475a; }
        QPushButton:pressed { background: #585b70; }
        QLineEdit   { background: #313244; color: #cdd6f4;
                      border: 1px solid #45475a; border-radius: 4px; padding: 4px; }
        QTextEdit   { background: #181825; color: #a6e3a1;
                      border: none; border-radius: 6px; }
        QLabel      { color: #cdd6f4; }
        QSplitter::handle { background: #313244; }
    )");

    // ── Central widget with horizontal splitter ──
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);

    // ── Left panel ──
    QWidget *leftPanel = new QWidget;
    leftPanel->setFixedWidth(270);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(8);
    leftLayout->setContentsMargins(12, 12, 12, 12);

    auto sectionLabel = [&](const QString &text) {
        QLabel *lbl = new QLabel(text);
        lbl->setStyleSheet("color:#89b4fa; font-weight:bold; font-size:13px;");
        return lbl;
    };

    // ── Add city ──
    leftLayout->addWidget(sectionLabel("Add City"));
    cityNameEdit = new QLineEdit; cityNameEdit->setPlaceholderText("City name…");
    leftLayout->addWidget(cityNameEdit);
    QPushButton *addCityBtn = new QPushButton("📍  Click Canvas to Place City");
    addCityBtn->setStyleSheet("background:#89b4fa; color:#1e1e2e; font-weight:bold;");
    leftLayout->addWidget(addCityBtn);

    // ── Edge weight ──
    leftLayout->addWidget(sectionLabel("Connect Cities"));
    QHBoxLayout *edgeRow1 = new QHBoxLayout;
    fromEdit = new QLineEdit; fromEdit->setPlaceholderText("From idx");
    toEdit   = new QLineEdit; toEdit->setPlaceholderText("To idx");
    edgeRow1->addWidget(fromEdit); edgeRow1->addWidget(toEdit);
    leftLayout->addLayout(edgeRow1);
    weightEdit = new QLineEdit; weightEdit->setPlaceholderText("Weight (distance)");
    leftLayout->addWidget(weightEdit);
    QPushButton *setEdgeBtn = new QPushButton("🔗  Set Edge");
    leftLayout->addWidget(setEdgeBtn);

    leftLayout->addSpacing(6);
    QFrame *sep1 = new QFrame; sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color:#45475a;"); leftLayout->addWidget(sep1);

    // ── Start city ──
    leftLayout->addWidget(sectionLabel("TSP Start City Index"));
    startEdit = new QLineEdit; startEdit->setText("0");
    leftLayout->addWidget(startEdit);

    QPushButton *bruteBtn  = new QPushButton("🔍  Brute Force TSP");
    QPushButton *greedyBtn = new QPushButton("⚡  Greedy TSP");
    QPushButton *compareBtn= new QPushButton("📊  Compare Both");
    bruteBtn->setStyleSheet("background:#a6e3a1; color:#1e1e2e; font-weight:bold;");
    greedyBtn->setStyleSheet("background:#a6e3a1; color:#1e1e2e; font-weight:bold;");
    compareBtn->setStyleSheet("background:#f9e2af; color:#1e1e2e; font-weight:bold;");
    leftLayout->addWidget(bruteBtn);
    leftLayout->addWidget(greedyBtn);
    leftLayout->addWidget(compareBtn);

    leftLayout->addSpacing(6);
    QFrame *sep2 = new QFrame; sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("color:#45475a;"); leftLayout->addWidget(sep2);

    // ── Dijkstra ──
    leftLayout->addWidget(sectionLabel("Shortest Path (Dijkstra)"));
    QHBoxLayout *dijkRow = new QHBoxLayout;
    srcEdit = new QLineEdit; srcEdit->setPlaceholderText("From");
    dstEdit = new QLineEdit; dstEdit->setPlaceholderText("To");
    dijkRow->addWidget(srcEdit); dijkRow->addWidget(dstEdit);
    leftLayout->addLayout(dijkRow);
    QPushButton *dijkBtn = new QPushButton("🗺️  Find Shortest Path");
    dijkBtn->setStyleSheet("background:#f38ba8; color:#1e1e2e; font-weight:bold;");
    leftLayout->addWidget(dijkBtn);

    leftLayout->addSpacing(6);
    QPushButton *clearBtn = new QPushButton("🗑️  Clear All");
    clearBtn->setStyleSheet("background:#585b70; color:#cdd6f4;");
    leftLayout->addWidget(clearBtn);

    // ── Animation speed ──
    leftLayout->addSpacing(4);
    leftLayout->addWidget(sectionLabel("Animation Speed"));
    QHBoxLayout *speedRow = new QHBoxLayout;
    QLabel *slowLbl = new QLabel("Slow");  slowLbl->setStyleSheet("color:#585b70; font-size:11px;");
    QLabel *fastLbl = new QLabel("Fast");  fastLbl->setStyleSheet("color:#585b70; font-size:11px;");
    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setMinimum(100);   // 100ms = fast
    speedSlider->setMaximum(1500);  // 1500ms = slow
    speedSlider->setValue(600);     // default 600ms
    speedSlider->setInvertedAppearance(true); // left=slow, right=fast
    speedSlider->setStyleSheet("QSlider::groove:horizontal { background:#313244; height:6px; border-radius:3px; }"
                               "QSlider::handle:horizontal { background:#89b4fa; width:14px; height:14px; border-radius:7px; margin:-4px 0; }");
    speedRow->addWidget(slowLbl);
    speedRow->addWidget(speedSlider);
    speedRow->addWidget(fastLbl);
    leftLayout->addLayout(speedRow);

    leftLayout->addStretch();

    // ── Right: canvas + output ──
    QWidget *rightPanel = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(8, 8, 8, 8);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 680, 460);
    scene->setBackgroundBrush(QBrush(QColor("#181825")));

    graphView = new QGraphicsView(scene);
    graphView->setRenderHint(QPainter::Antialiasing);
    graphView->setDragMode(QGraphicsView::NoDrag);
    graphView->setStyleSheet("border: 2px solid #313244; border-radius:8px;");
    graphView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    outputEdit = new QTextEdit;
    outputEdit->setReadOnly(true);
    outputEdit->setFixedHeight(140);
    outputEdit->setStyleSheet("background:#181825; color:#a6e3a1; "
                              "font-family:'Courier New'; font-size:12px; border-radius:6px;");

    QLabel *canvasLabel = new QLabel("🗺️  Graph Canvas  —  click to place cities");
    canvasLabel->setStyleSheet("color:#585b70; font-size:12px;");

    rightLayout->addWidget(canvasLabel);
    rightLayout->addWidget(graphView, 1);
    rightLayout->addWidget(new QLabel("Output Log"));
    rightLayout->addWidget(outputEdit);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(1, 1);

    // ── Connect signals ──
    connect(addCityBtn, &QPushButton::clicked, this, &MainWindow::onAddCity);
    connect(setEdgeBtn, &QPushButton::clicked, this, &MainWindow::onSetEdge);
    connect(bruteBtn,   &QPushButton::clicked, this, &MainWindow::onRunBrute);
    connect(greedyBtn,  &QPushButton::clicked, this, &MainWindow::onRunGreedy);
    connect(dijkBtn,    &QPushButton::clicked, this, &MainWindow::onRunDijkstra);
    connect(compareBtn, &QPushButton::clicked, this, &MainWindow::onCompare);
    connect(clearBtn,   &QPushButton::clicked, this, &MainWindow::onClearAll);

    // Click on canvas to place city
    connect(scene, &QGraphicsScene::changed, this, []{});
    graphView->viewport()->installEventFilter(this);

    memset(adjMatrix, 0, sizeof(adjMatrix));
    log("Welcome! Add cities by typing a name and clicking the canvas.");
}

MainWindow::~MainWindow() { delete ui; }

// ─────────────────────────────────────────────────────
//  Event filter — catch canvas mouse clicks
// ─────────────────────────────────────────────────────
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == graphView->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton && placingCity) {
            QPointF scenePos = graphView->mapToScene(me->pos());
            onCanvasClicked(scenePos);
            placingCity = false;
            graphView->setCursor(Qt::ArrowCursor);
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// ─────────────────────────────────────────────────────
//  Slots
// ─────────────────────────────────────────────────────
void MainWindow::onAddCity()
{
    if (numCities >= 20) { log("Max 20 cities."); return; }
    QString name = cityNameEdit->text().trimmed();
    if (name.isEmpty()) name = QString("City%1").arg(numCities);
    pendingCityName = name;
    placingCity = true;
    graphView->setCursor(Qt::CrossCursor);
    log(QString("Click the canvas to place \"%1\"").arg(name));
}

void MainWindow::onCanvasClicked(QPointF pos)
{
    CityNode city;
    city.name = pendingCityName;
    city.pos  = pos;
    cities.append(city);

    // Draw pin circle
    const int R = 18;
    QGraphicsEllipseItem *circle = scene->addEllipse(
        pos.x()-R, pos.y()-R, R*2, R*2,
        QPen(QColor("#89b4fa"), 2),
        QBrush(QColor("#313244"))
    );
    circle->setZValue(2);
    cityCircles.append(circle);

    // City index label inside circle
    QGraphicsTextItem *idxLabel = scene->addText(QString::number(numCities));
    idxLabel->setDefaultTextColor(QColor("#cdd6f4"));
    idxLabel->setFont(QFont("Segoe UI", 9, QFont::Bold));
    idxLabel->setPos(pos.x() - 7, pos.y() - 10);
    idxLabel->setZValue(3);

    // City name label below
    QGraphicsTextItem *nameLabel = scene->addText(city.name);
    nameLabel->setDefaultTextColor(QColor("#89b4fa"));
    nameLabel->setFont(QFont("Segoe UI", 9));
    nameLabel->setPos(pos.x() - 20, pos.y() + R + 2);
    nameLabel->setZValue(3);
    cityLabels.append(nameLabel);

    numCities++;
    cityNameEdit->clear();
    log(QString("[%1] %2 placed at (%3, %4)")
        .arg(numCities-1).arg(city.name)
        .arg((int)pos.x()).arg((int)pos.y()));
}

void MainWindow::onSetEdge()
{
    bool ok1, ok2, ok3;
    int from   = fromEdit->text().toInt(&ok1);
    int to     = toEdit->text().toInt(&ok2);
    int weight = weightEdit->text().toInt(&ok3);

    if (!ok1 || !ok2 || !ok3 || from < 0 || to < 0 ||
        from >= numCities || to >= numCities || weight <= 0) {
        log("Invalid edge. Check city indices and weight."); return;
    }
    if (from == to) { log("Self-loops not allowed."); return; }

    adjMatrix[from][to] = weight;
    adjMatrix[to][from] = weight; // undirected

    redrawGraph();
    log(QString("Edge %1 ↔ %2, weight = %3")
        .arg(cities[from].name).arg(cities[to].name).arg(weight));

    fromEdit->clear(); toEdit->clear(); weightEdit->clear();
}

void MainWindow::onRunBrute()
{
    if (numCities < 2) { log("Need at least 2 cities."); return; }
    int start = startEdit->text().toInt();
    if (start < 0 || start >= numCities) start = 0;

    QVector<int> route = bruteForceTSP(start);

    if (route.isEmpty()) {
        log("=== Brute Force ===");
        log("No complete route found — some edges are missing.");
        log("Make sure all cities are connected to each other.");
        return;
    }

    int cost = routeCost(route);

    clearHighlights();
    highlightPath(route, true, QColor("#a6e3a1"));

    QString r;
    for (int i = 0; i < route.size(); i++) {
        r += cities[route[i]].name;
        if (i < route.size()-1) r += " → ";
    }
    r += " → " + cities[route[0]].name;
    log("=== Brute Force ===");
    log(r);
    log("Cost: " + QString::number(cost));
}

void MainWindow::onRunGreedy()
{
    if (numCities < 2) { log("Need at least 2 cities."); return; }
    int start = startEdit->text().toInt();
    if (start < 0 || start >= numCities) start = 0;

    QVector<int> route = greedyTSP(start);
    int cost = routeCost(route);

    clearHighlights();
    highlightPath(route, true, QColor("#fab387"));

    QString r;
    for (int i = 0; i < route.size(); i++) {
        r += cities[route[i]].name;
        if (i < route.size()-1) r += " → ";
    }
    r += " → " + cities[route[0]].name;
    log("=== Greedy ===");
    log(r);
    log("Cost: " + QString::number(cost));
}

void MainWindow::onRunDijkstra()
{
    bool ok1, ok2;
    int src = srcEdit->text().toInt(&ok1);
    int dst = dstEdit->text().toInt(&ok2);
    if (!ok1 || !ok2 || src < 0 || dst < 0 || src >= numCities || dst >= numCities) {
        log("Invalid source/destination."); return;
    }
    if (src == dst) { log("Source and destination are the same."); return; }

    int totalCost = 0;
    QVector<int> path = dijkstra(src, dst, totalCost);

    if (path.isEmpty()) {
        log(QString("No path found from %1 to %2.").arg(cities[src].name).arg(cities[dst].name));
        return;
    }

    clearHighlights();
    highlightPath(path, false, QColor("#f38ba8"));

    QString r;
    for (int i = 0; i < path.size(); i++) {
        r += cities[path[i]].name;
        if (i < path.size()-1) r += " → ";
    }
    log("=== Dijkstra Shortest Path ===");
    log(r);
    log("Total Cost: " + QString::number(totalCost));
}

void MainWindow::onCompare()
{
    if (numCities < 2) { log("Need at least 2 cities."); return; }
    int start = startEdit->text().toInt();
    if (start < 0 || start >= numCities) start = 0;

    QVector<int> r1 = bruteForceTSP(start);
    QVector<int> r2 = greedyTSP(start);

    log("========== Comparison ==========");
    log(QString("Brute Force  → Cost: %1").arg(routeCost(r1)));
    log(QString("Greedy       → Cost: %1").arg(routeCost(r2)));
    log("================================");
}

void MainWindow::onClearAll()
{
    scene->clear();
    cities.clear();
    cityCircles.clear();
    cityLabels.clear();
    edgeLines.clear();
    edgeWeightLabels.clear();
    highlightItems.clear();
    numCities = 0;
    memset(adjMatrix, 0, sizeof(adjMatrix));
    outputEdit->clear();
    log("Cleared. Start fresh!");
}

// ─────────────────────────────────────────────────────
//  Graph drawing
// ─────────────────────────────────────────────────────
void MainWindow::redrawGraph()
{
    // Remove old edges
    for (auto *item : edgeLines)        scene->removeItem(item);
    for (auto *item : edgeWeightLabels) scene->removeItem(item);
    edgeLines.clear();
    edgeWeightLabels.clear();

    for (int i = 0; i < numCities; i++) {
        for (int j = i+1; j < numCities; j++) {
            if (adjMatrix[i][j] <= 0) continue;

            QPointF a = cities[i].pos;
            QPointF b = cities[j].pos;

            QGraphicsLineItem *line = scene->addLine(
                a.x(), a.y(), b.x(), b.y(),
                QPen(QColor("#45475a"), 2)
            );
            line->setZValue(1);
            edgeLines.append(line);

            // Weight label at midpoint
            QPointF mid((a.x()+b.x())/2, (a.y()+b.y())/2);
            QGraphicsTextItem *wlbl = scene->addText(QString::number(adjMatrix[i][j]));
            wlbl->setDefaultTextColor(QColor("#585b70"));
            wlbl->setFont(QFont("Segoe UI", 8));
            wlbl->setPos(mid.x()+4, mid.y()-10);
            wlbl->setZValue(1);
            edgeWeightLabels.append(wlbl);
        }
    }
}

void MainWindow::highlightPath(const QVector<int>& path, bool loop, QColor color)
{
    // Build the list of steps: each step is a pair of city indices
    QVector<QPair<int,int>> steps;
    for (int i = 0; i < path.size() - 1; i++)
        steps.append({path[i], path[i+1]});
    if (loop && path.size() > 1)
        steps.append({path.last(), path.first()});

    if (steps.isEmpty()) return;

    // Highlight the starting node immediately
    auto pulseCity = [&](int idx, QColor c) {
        QPointF p = cities[idx].pos;
        const int R = 18;
        QGraphicsEllipseItem *ring = scene->addEllipse(
            p.x()-R, p.y()-R, R*2, R*2,
            QPen(c, 3), QBrush(Qt::transparent)
        );
        ring->setZValue(2.5);
        highlightItems.append(ring);
    };

    pulseCity(path.first(), color);

    // Use a QTimer to draw one edge per tick (600ms apart)
    int *step = new int(0);
    QTimer *timer = new QTimer(this);
    timer->setInterval(speedSlider->value()); // controlled by slider

    connect(timer, &QTimer::timeout, this, [=]() mutable {
        if (*step >= steps.size()) {
            timer->stop();
            timer->deleteLater();
            delete step;
            log("Animation complete.");
            return;
        }

        int a = steps[*step].first;
        int b = steps[*step].second;

        // Draw the edge line
        QPointF pa = cities[a].pos;
        QPointF pb = cities[b].pos;
        QGraphicsLineItem *line = scene->addLine(
            pa.x(), pa.y(), pb.x(), pb.y(),
            QPen(color, 4, Qt::SolidLine, Qt::RoundCap)
        );
        line->setZValue(1.5);
        highlightItems.append(line);

        // Pulse the destination city
        pulseCity(b, color);

        log(QString("  Step %1: %2 → %3  (weight: %4)")
            .arg(*step + 1)
            .arg(cities[a].name)
            .arg(cities[b].name)
            .arg(adjMatrix[a][b]));

        (*step)++;
    });

    timer->start();
}

void MainWindow::clearHighlights()
{
    for (auto *item : highlightItems)
        scene->removeItem(item);
    highlightItems.clear();
}

// ─────────────────────────────────────────────────────
//  Algorithms
// ─────────────────────────────────────────────────────
int MainWindow::routeCost(const QVector<int>& route)
{
    int cost = 0;
    for (int i = 0; i < route.size()-1; i++)
        cost += adjMatrix[route[i]][route[i+1]];
    cost += adjMatrix[route.last()][route.first()];
    return cost;
}

QVector<int> MainWindow::bruteForceTSP(int start)
{
    QVector<int> cities_idx;
    for (int i = 0; i < numCities; i++)
        if (i != start) cities_idx.append(i);

    std::sort(cities_idx.begin(), cities_idx.end());

    int bestCost = 999999999;
    QVector<int> bestRoute;

    do {
        QVector<int> route = {start};
        route += cities_idx;

        // Check all edges exist (no zero-weight gaps)
        bool valid = true;
        for (int i = 0; i < route.size() - 1; i++) {
            if (adjMatrix[route[i]][route[i+1]] == 0) { valid = false; break; }
        }
        // Also check return edge
        if (adjMatrix[route.last()][route.first()] == 0) valid = false;

        if (!valid) continue;

        int cost = routeCost(route);
        if (cost < bestCost) { bestCost = cost; bestRoute = route; }

    } while (std::next_permutation(cities_idx.begin(), cities_idx.end()));

    return bestRoute;
}

QVector<int> MainWindow::greedyTSP(int start)
{
    QVector<bool> visited(numCities, false);
    QVector<int> route;
    int current = start;
    route.append(current);
    visited[current] = true;

    for (int step = 1; step < numCities; step++) {
        int nearest = -1, minDist = INT_MAX;
        for (int j = 0; j < numCities; j++) {
            if (!visited[j] && adjMatrix[current][j] > 0 && adjMatrix[current][j] < minDist) {
                minDist = adjMatrix[current][j];
                nearest = j;
            }
        }
        if (nearest == -1) break;
        route.append(nearest);
        visited[nearest] = true;
        current = nearest;
    }
    return route;
}

QVector<int> MainWindow::dijkstra(int src, int dst, int &totalCost)
{
    const int INF = 999999999;
    QVector<int> dist(numCities, INF);
    QVector<int> prev(numCities, -1);
    using pii = std::pair<int,int>;
    std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pq;

    dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (int v = 0; v < numCities; v++) {
            if (v == u || adjMatrix[u][v] <= 0 || dist[u] == INF) continue;
            int nd = dist[u] + adjMatrix[u][v];
            if (nd < dist[v]) {
                dist[v] = nd;
                prev[v] = u;
                pq.push({nd, v});
            }
        }
    }

    if (dist[dst] == INF) { totalCost = 0; return {}; }

    totalCost = dist[dst];
    QVector<int> path;
    for (int at = dst; at != -1; at = prev[at])
        path.prepend(at);
    return path;
}

void MainWindow::log(const QString &msg)
{
    outputEdit->append(msg);
}
