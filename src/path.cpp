#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
#include "path.h"

using namespace std;
static int id = 0;
// height - длина вертикального столбца, width - горизонтальной строчки

ShortestPath::ShortestPath(int wid, int hei, int step) : step(step) {
    width = wid / step;
    height = hei / step;
    size = width * height;
    occupied.resize(size + 1);
    fill(occupied.begin(), occupied.end(), 0);
    data.resize(size + 1);
    pathCoord.resize(width);
    rectData.resize(width);
}

void ShortestPath::fillGraph() { // нумерация построчно, начиная с единицы
    vector<int> used(size, 0);

    for (int i = 0; i < size; i++) {
        data[i].push_back(i);
    }

    /* углы: левый верх, правый верх, левый низ, правый низ */

    data[0].push_back(data[1][0]); // справа
    data[0].push_back(data[height][0]); // под
    used[0] = 1;

    data[width - 1].push_back(data[width - 2][0]); // слева
    data[width - 1].push_back(data[width + width - 1][0]); // под
    used[width - 1] = 1;

    data[size - height].push_back(data[size - height + 1][0]); // справа
    data[size - height].push_back(data[size - height - height][0]); // над
    used[size - height] = 1;

    data[size - 1].push_back(data[size - 2][0]); // слева
    data[size - 1].push_back(data[size - height - 1][0]); // над
    used[size - 1] = 1;

    for (int i = 1; i < width - 1; i++) { // верхняя строчка
        data[i].push_back(data[i + 1][0]); // справа
        data[i].push_back(data[i - 1][0]); // слева
        data[i].push_back(data[i + height][0]); // снизу !?
        used[i] = 1;
    }

    for (int i = width; i != size - width; i += width) { // левый столбец
        data[i].push_back(data[i + 1][0]); // справа
        data[i].push_back(data[i - width][0]); // сверху
        data[i].push_back(data[i + width][0]); // снизу
        used[i] = 1;
    }

    for (int i = width + width - 1; i < size - 1; i += width) { // правый столбец
        data[i].push_back(data[i - 1][0]); // слева
        data[i].push_back(data[i - width][0]); // сверху
        data[i].push_back(data[i + width][0]); // снизу
        used[i] = 1;
    }

    for (int i = size - width + 1; i < size - 1; i++) { // нижняя строчка
        data[i].push_back(data[i + 1][0]); // справа
        data[i].push_back(data[i - 1][0]); // слева
        data[i].push_back(data[i - width][0]); // сверху
        used[i] = 1;
    }

    for (int i = width + 1; i < size - width - 1; i++) { // все остальные вершины
        if (!used[i]) {
            data[i].push_back(data[i + 1][0]); // справа
            data[i].push_back(data[i - 1][0]); // слева
            data[i].push_back(data[i - width][0]); // сверху
            data[i].push_back(data[i + width][0]); // снизу
        }
    }
}

void ShortestPath::showGraph() {
    int cnt = width - 1;
    char graph[size + 1];
    for (int i = 0; i < size; i++) {
        occupied[i] ? graph[i] = 'x' : graph[i] = '.';
    }
    for (auto i : path) {
        graph[i] = '~';
    }
    for (int i = 0; i < size; i++) {
        if (i > cnt) {
            cout << '\n';
            cnt += width;
        }
        cout << ' ' << graph[i] << ' ';
    }
}

pair<int, int> ShortestPath::convertToPair(int x) {
    int w = x / width, h = x % width;
    return make_pair(w, h); // нумерация с нуля
}

int ShortestPath::convertToNum(pair<int, int> coord) {
    return coord.first * width + coord.second;
}

int ShortestPath::convertToStep(int x) {
    int res;
    res = x / step + (x % step != 0);
    return res;
}

int ShortestPath::convertFromStep(int x) {
    return x * step;
}

rectangle ShortestPath::getRect(int id, pair<int, int> centre_coord, int w, int h) {
    int centre = convertToStep(convertToNum(centre_coord));
    int wid = convertToStep(w);
    int hei = convertToStep(h);
    rectangle rect(id, centre, wid, hei); // id передается в функицю
    rectData[id] = rect;
    return rect;
}

void ShortestPath::getRectCoord(int x, int y, int w, int h) {
    shapeId[{x, y}] = id;
    rectData[id] = getRect(id, make_pair(x, y), w, h);
    addShape(rectData[id]);
    ++id;
}

void ShortestPath::addShape(rectangle &rect) {
    long long left_up = rect.getCentre() - rect.getWidth() / 2 - width * (rect.getHeight() / 2);
    long long right_up = rect.getCentre() + rect.getWidth() / 2 - width * (rect.getHeight() / 2); // тут погрешность
    int cnt = 0;
    for (int i = 0; i < rect.getHeight(); i++) {
        for (int j = left_up + width * i; j < right_up + width * i; j++) {
            assert(j <= size);
            occupied[j] = 1;
            assert(cnt <= rect.data.size());
            rect.data[cnt++] = j;
        }
    }
}

void ShortestPath::deleteShape(const rectangle &rect) { // точно удаляет все, что нужно, ура
    for (auto i : rect.data) {
        occupied[i] = 0;
    }
}

void ShortestPath::dragShape(int id, pair<int, int> new_centre) {
    deleteShape(rectData[id]);
    int new_converted_centre = convertToStep(convertToNum(new_centre));
    rectangle new_rect(rectData[id].getId(), new_converted_centre, rectData[id].getWidth(), rectData[id].getHeight());
    rectData[id] = new_rect;
    addShape(rectData[id]);
}

pair<int, int> ShortestPath::findBorderPoint(int id1, int id2) {
    int coord1, coord2;
    rectangle rect1 = getRectData(id1);
    rectangle rect2 = getRectData(id2);
    pair<int, int> cent1 = convertToPair(rect1.getCentre());
    pair<int, int> cent2 = convertToPair(rect2.getCentre());
    if (cent1.first < cent2.first) {
        coord1 = rect1.getCentre() + rect1.getWidth() / 2;
        coord2 = rect2.getCentre() - rect2.getWidth() / 2;
    } else if (cent1.first > cent2.first) {
        coord1 = rect1.getCentre() - rect1.getWidth() / 2;
        coord2 = rect2.getCentre() + rect2.getWidth() / 2;
    } else if (cent1.first == cent2.first) {
        coord1 = rect1.getCentre() + rect1.getWidth() / 2;
        coord2 = rect2.getCentre() + rect2.getWidth() / 2;
    }
    return make_pair(convertToStep(coord1), convertToStep(coord2));
}

vector<pair<int, int>> ShortestPath::createShortestPath(int x1, int y1, int x2, int y2) {
    int id1 = shapeId[{x1, y1}];
    int id2 = shapeId[{x2, y2}];
    int src = findBorderPoint(id1, id2).first;
    int dest = findBorderPoint(id1, id2).second;
    queue<int> q;
    vector<int> dist(size, -1);
    vector<bool> is(size, false);
    int par[size];
    is[src] = true;
    dist[src] = 0;
    par[src] = -1;
    q.push(src);
    while (!q.empty()) {
        int cur = q.front();
        q.pop();
        for (int i = 0; i < data[cur].size(); i++) {
            int u = data[cur][i];
            if (!is[u] && (!occupied[u] || u == dest)) {
                is[u] = true;
                q.push(u);
                dist[u] = dist[cur] + 1;
                par[u] = cur;
            }
        }
    }

    vector<pair<int, int>> ans;
    for (int i = dest; i != -1; i = par[i]) {
        ans.push_back(convertToPair(convertFromStep(i)));
        //occupied[i] = 1;
    }
    return ans;
}