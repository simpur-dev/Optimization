#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <cmath>
using namespace std;

// ====================== 实验参数设置 ======================
const int city_num = 10;                // 节点数量（固定10个）
const int pop_size = 50;                 // 种群规模
const int max_iter = 200;                 // 最大迭代次数
const double cross_prob = 0.8;            // 交叉概率
const double mutate_prob = 0.1;           // 变异概率

// ====================== 10节点距离邻接矩阵（替换为你的实验数据） ======================
int dist[city_num][city_num] = {
    {0,    11,    6,    5,    3,   15,    1,    3,   16,    2},
    {11,    0,    2,   17,    8,    5, 9999, 9999,    2,    2},
    {6,     2,    0,    3,    4, 9999, 9999, 9999,    4,    4},
    {5,    17,    3,    0,    3, 9999, 9999, 9999,    8,    3},
    {3,     8,    4,    3,    0, 9999, 9999, 9999,    3,   13},
    {15,    5, 9999, 9999, 9999,    0, 9999,   10, 9999,    1},
    {1,  9999, 9999, 9999, 9999, 9999,    0, 9999, 9999,    4},
    {3,  9999, 9999, 9999, 9999,   10, 9999,    0, 9999,    3},
    {16,    2,    4,    8,    3, 9999, 9999, 9999,    0,   12},
    {2,     2,    4,    3,   13,    1,    4,    3,   12,    0}
};
#define INF 9999  // 不可达的无穷大值

// ====================== 个体结构体（一条路径=一个染色体） ======================
struct Individual {
    vector<int> path;       // 路径（1~10的排列）
    double fitness;         // 适应度
    int total_dist;         // 路径总长度
};

// ====================== 工具函数：计算路径总长度 ======================
int calc_total_dist(const vector<int>& path) {
    int total = 0;
    for (int i = 0; i < city_num - 1; i++) {
        int from = path[i] - 1;    // 转成0索引
        int to = path[i+1] - 1;
        if (dist[from][to] == INF) return INF; // 不可达路径，直接返回无穷大
        total += dist[from][to];
    }
    // 回到起点，形成闭合回路
    int last = path.back() - 1;
    int first = path[0] - 1;
    if (dist[last][first] == INF) return INF;
    total += dist[last][first];
    return total;
}

// ====================== 1. 初始化种群 ======================
void init_population(vector<Individual>& pop) {
    // 基础路径1~10
    vector<int> base_path(city_num);
    for (int i = 0; i < city_num; i++) base_path[i] = i + 1;

    for (int i = 0; i < pop_size; i++) {
        Individual ind;
        ind.path = base_path;
        random_shuffle(ind.path.begin(), ind.path.end()); // 随机打乱，生成合法路径
        ind.total_dist = calc_total_dist(ind.path);
        // 处理不可达路径，重新生成
        while (ind.total_dist == INF) {
            random_shuffle(ind.path.begin(), ind.path.end());
            ind.total_dist = calc_total_dist(ind.path);
        }
        ind.fitness = 1.0 / ind.total_dist; // 适应度=1/总长度
        pop.push_back(ind);
    }
}

// ====================== 2. 选择操作：轮盘赌+精英保留 ======================
void select(vector<Individual>& pop) {
    vector<Individual> new_pop;
    // 1. 精英保留：直接保留最优的2个个体
    sort(pop.begin(), pop.end(), [](const Individual& a, const Individual& b) {
        return a.fitness > b.fitness;
    });
    new_pop.push_back(pop[0]);
    new_pop.push_back(pop[1]);

    // 2. 轮盘赌选择剩下的个体
    double total_fitness = 0;
    for (auto& ind : pop) total_fitness += ind.fitness;

    // 生成轮盘
    vector<double> roulette(pop_size);
    roulette[0] = pop[0].fitness / total_fitness;
    for (int i = 1; i < pop_size; i++) {
        roulette[i] = roulette[i-1] + pop[i].fitness / total_fitness;
    }

    // 轮盘赌选择
    for (int i = 2; i < pop_size; i++) {
        double r = (double)rand() / RAND_MAX;
        for (int j = 0; j < pop_size; j++) {
            if (r <= roulette[j]) {
                new_pop.push_back(pop[j]);
                break;
            }
        }
    }

    pop = new_pop;
}

// ====================== 3. 交叉操作：顺序交叉(OX) ======================
void cross(vector<Individual>& pop) {
    for (int i = 0; i < pop_size - 1; i += 2) {
        // 按交叉概率判断是否交叉
        double r = (double)rand() / RAND_MAX;
        if (r > cross_prob) continue;

        Individual& parent1 = pop[i];
        Individual& parent2 = pop[i+1];
        vector<int> child1(city_num), child2(city_num);

        // 随机选择两个交叉点
        int start = rand() % city_num;
        int end = rand() % city_num;
        if (start > end) swap(start, end);

        // 1. 保留父代的中间片段
        vector<bool> used1(city_num + 1, false), used2(city_num + 1, false);
        for (int j = start; j <= end; j++) {
            child1[j] = parent1.path[j];
            used1[child1[j]] = true;
            child2[j] = parent2.path[j];
            used2[child2[j]] = true;
        }

        // 2. 填充剩余位置：从父代的end+1开始，依次填入不重复的节点
        int idx1 = (end + 1) % city_num;
        int idx2 = (end + 1) % city_num;
        for (int j = (end + 1) % city_num; j != start; j = (j + 1) % city_num) {
            // 填充child1
            while (used1[parent2.path[idx2]]) idx2 = (idx2 + 1) % city_num;
            child1[j] = parent2.path[idx2];
            used1[child1[j]] = true;
            idx2 = (idx2 + 1) % city_num;

            // 填充child2
            while (used2[parent1.path[idx1]]) idx1 = (idx1 + 1) % city_num;
            child2[j] = parent1.path[idx1];
            used2[child2[j]] = true;
            idx1 = (idx1 + 1) % city_num;
        }

        // 3. 更新子代，计算适应度
        parent1.path = child1;
        parent1.total_dist = calc_total_dist(child1);
        if (parent1.total_dist != INF) parent1.fitness = 1.0 / parent1.total_dist;

        parent2.path = child2;
        parent2.total_dist = calc_total_dist(child2);
        if (parent2.total_dist != INF) parent2.fitness = 1.0 / parent2.total_dist;
    }
}

// ====================== 4. 变异操作：交换变异 ======================
void mutate(vector<Individual>& pop) {
    for (int i = 0; i < pop_size; i++) {
        // 按变异概率判断是否变异
        double r = (double)rand() / RAND_MAX;
        if (r > mutate_prob) continue;

        // 随机选两个位置交换
        int pos1 = rand() % city_num;
        int pos2 = rand() % city_num;
        while (pos1 == pos2) pos2 = rand() % city_num;
        swap(pop[i].path[pos1], pop[i].path[pos2]);

        // 重新计算适应度
        pop[i].total_dist = calc_total_dist(pop[i].path);
        if (pop[i].total_dist != INF) pop[i].fitness = 1.0 / pop[i].total_dist;
    }
}

// ====================== 主函数：算法主流程 ======================
int main() {
    srand((unsigned int)time(NULL)); // 初始化随机数种子

    vector<Individual> pop;
    init_population(pop); // 初始化种群

    // 记录全局最优解
    Individual global_best = pop[0];
    int best_iter = 0;

    cout << "===== 遗传算法求解10节点最短路径问题 =====" << endl;
    cout << "迭代次数\t当前最优路径长度" << endl;

    // 主迭代循环
    for (int iter = 0; iter < max_iter; iter++) {
        select(pop);   // 选择
        cross(pop);    // 交叉
        mutate(pop);   // 变异

        // 更新当前代最优解
        sort(pop.begin(), pop.end(), [](const Individual& a, const Individual& b) {
            return a.fitness > b.fitness;
        });
        Individual current_best = pop[0];

        // 更新全局最优解
        if (current_best.fitness > global_best.fitness) {
            global_best = current_best;
            best_iter = iter + 1;
        }

        // 每10代输出一次进度
        if ((iter + 1) % 10 == 0) {
            cout << setw(6) << iter + 1 << "\t\t" << current_best.total_dist << endl;
        }
    }

    // 输出最终结果
    cout << "\n===== 算法执行完成，最终结果 =====" << endl;
    cout << "全局最短路径长度：" << global_best.total_dist << endl;
    cout << "最优路径出现迭代：" << best_iter << endl;
    cout << "最短路径（闭合回路）：";
    for (int num : global_best.path) cout << num << " → ";
    cout << global_best.path[0] << endl;

    return 0;
}