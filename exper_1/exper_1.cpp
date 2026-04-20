#include <iostream>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <string>
using namespace std;

// ====================== 1. 通用定义 ======================
// 粒子结构体：通用，适配任意二维函数
struct Particle
{
    double x1, x2;               // 粒子位置（二维）
    double v1, v2;               // 粒子速度
    double pbest_x1, pbest_x2;   // 个体历史最优位置pbest
    double pbest_fit;             // 个体历史最优适应度
};

// ====================== 2. 目标函数定义 ======================
// 二维Griewank函数
double Griewank(double x1, double x2)
{
    double sum_sq = x1 * x1 + x2 * x2;
    double prod_cos = cos(x1 / sqrt(1.0)) * cos(x2 / sqrt(2.0));
    return 1.0 + sum_sq / 4000.0 - prod_cos;
}

// 二维Rastrigin函数
double Rastrigin(double x1, double x2)
{
    double sum_sq = x1 * x1 + x2 * x2;
    double sum_cos = cos(2 * M_PI * x1) + cos(2 * M_PI * x2);
    return 20.0 + sum_sq - 10.0 * sum_cos;
}

// ====================== 3. 通用PSO算法封装 ======================
// 参数说明：
// func: 目标函数指针（可以传入Griewank或Rastrigin）
// x_min, x_max: 位置搜索范围
// func_name: 函数名称（用于输出显示）
void runPSO(double (*func)(double, double), double x_min, double x_max, const string& func_name)
{
    // ---------------------- PSO算法参数设置 ----------------------
    const int pop_size = 50;          // 种群规模
    const int max_iter = 1000;        // 最大迭代次数
    const double c1 = 2.0;            // 个体学习因子
    const double c2 = 2.0;            // 全局学习因子
    const double w_max = 0.9;         // 最大惯性权重
    const double w_min = 0.4;         // 最小惯性权重
    const double v_max = 0.2 * (x_max - x_min); // 速度上限
    const double v_min = -v_max;      // 速度下限

    // ---------------------- 初始化全局最优 ----------------------
    double gbest_x1, gbest_x2;
    double gbest_fit = 1e10;

    // ---------------------- 初始化粒子群 ----------------------
    Particle pop[pop_size];
    for (int i = 0; i < pop_size; i++)
    {
        // 随机初始化位置
        pop[i].x1 = x_min + (x_max - x_min) * (double)rand() / RAND_MAX;
        pop[i].x2 = x_min + (x_max - x_min) * (double)rand() / RAND_MAX;

        // 随机初始化速度
        pop[i].v1 = v_min + (v_max - v_min) * (double)rand() / RAND_MAX;
        pop[i].v2 = v_min + (v_max - v_min) * (double)rand() / RAND_MAX;

        // 计算初始适应度
        double init_fit = func(pop[i].x1, pop[i].x2);

        // 初始化个体最优
        pop[i].pbest_x1 = pop[i].x1;
        pop[i].pbest_x2 = pop[i].x2;
        pop[i].pbest_fit = init_fit;

        // 更新全局最优
        if (pop[i].pbest_fit < gbest_fit)
        {
            gbest_fit = pop[i].pbest_fit;
            gbest_x1 = pop[i].pbest_x1;
            gbest_x2 = pop[i].pbest_x2;
        }
    }

    // ---------------------- PSO迭代优化 ----------------------
    cout << "\n========== 开始优化 " << func_name << " 函数 ==========" << endl;
    for (int iter = 0; iter < max_iter; iter++)
    {
        // 惯性权重线性递减
        double w = w_max - (w_max - w_min) * (double)iter / max_iter;

        // 遍历所有粒子
        for (int i = 0; i < pop_size; i++)
        {
            // 生成随机数
            double r1 = (double)rand() / RAND_MAX;
            double r2 = (double)rand() / RAND_MAX;

            // 速度更新
            pop[i].v1 = w * pop[i].v1 + c1 * r1 * (pop[i].pbest_x1 - pop[i].x1) + c2 * r2 * (gbest_x1 - pop[i].x1);
            pop[i].v2 = w * pop[i].v2 + c1 * r1 * (pop[i].pbest_x2 - pop[i].x2) + c2 * r2 * (gbest_x2 - pop[i].x2);

            // 速度越界限制
            pop[i].v1 = (pop[i].v1 > v_max) ? v_max : pop[i].v1;
            pop[i].v1 = (pop[i].v1 < v_min) ? v_min : pop[i].v1;
            pop[i].v2 = (pop[i].v2 > v_max) ? v_max : pop[i].v2;
            pop[i].v2 = (pop[i].v2 < v_min) ? v_min : pop[i].v2;

            // 位置更新
            pop[i].x1 += pop[i].v1;
            pop[i].x2 += pop[i].v2;

            // 位置越界限制
            pop[i].x1 = (pop[i].x1 > x_max) ? x_max : pop[i].x1;
            pop[i].x1 = (pop[i].x1 < x_min) ? x_min : pop[i].x1;
            pop[i].x2 = (pop[i].x2 > x_max) ? x_max : pop[i].x2;
            pop[i].x2 = (pop[i].x2 < x_min) ? x_min : pop[i].x2;

            // 更新个体最优
            double current_fit = func(pop[i].x1, pop[i].x2);
            if (current_fit < pop[i].pbest_fit)
            {
                pop[i].pbest_x1 = pop[i].x1;
                pop[i].pbest_x2 = pop[i].x2;
                pop[i].pbest_fit = current_fit;
            }

            // 更新全局最优
            if (pop[i].pbest_fit < gbest_fit)
            {
                gbest_fit = pop[i].pbest_fit;
                gbest_x1 = pop[i].pbest_x1;
                gbest_x2 = pop[i].pbest_x2;
            }
        }

        // 每100次迭代输出进度
        if ((iter + 1) % 100 == 0)
        {
            cout << "迭代次数：" << setw(4) << iter + 1
                 << "  当前最优函数值：" << fixed << setprecision(10) << gbest_fit << endl;
        }
    }

    // ---------------------- 输出最终结果 ----------------------
    cout << "\n===== " << func_name << " 函数优化最终结果 =====" << endl;
    cout << "全局最优位置 x1 = " << fixed << setprecision(10) << gbest_x1 << endl;
    cout << "全局最优位置 x2 = " << fixed << setprecision(10) << gbest_x2 << endl;
    cout << "函数最小值 = " << fixed << setprecision(10) << gbest_fit << endl;
    cout << "理论全局最小值 = 0.0" << endl;
}

// ====================== 4. 主函数 ======================
int main()
{
    // 初始化随机数种子（只需初始化一次）
    srand((unsigned int)time(NULL));

    // 1. 优化Griewank函数
    runPSO(Griewank, -600.0, 600.0, "Griewank");

    // 2. 优化Rastrigin函数
    runPSO(Rastrigin, -5.12, 5.12, "Rastrigin");

    return 0;
}