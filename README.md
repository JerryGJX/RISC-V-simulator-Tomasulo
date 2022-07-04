# RISC-V-simulator-Tomasulo
A simple CPU simulator with Tomasulo

## 逻辑图
![tomasulo_frame](images/tomasulo_frame.jpg)

## 关于分支预测

使用4096个二位饱和计数器。预测的准确率如下：

| 测试点         | 分支总数 | 成功预测数 | 成功率  |
| -------------- | :------: | :--------: | :-----: |
| array_test1    |    22    |     12     | 54.55%  |
| array_test2    |    26    |     15     | 57.69%  |
| basicopt1      |  155139  |   127840   | 82.40%  |
| bulgarian      |  71493   |   67527    | 94.45%  |
| expr           |   111    |     94     | 84.68%  |
| gcd            |   120    |     81     | 67.50%  |
| hanoi          |  17457   |   10667    | 61.10%  |
| lvalue2        |    6     |     4      | 66.67%  |
| magic          |  67869   |   53220    | 78.42%  |
| manyarguments  |    10    |     6      | 60.00%  |
| multiarray     |   162    |    135     | 83.33%  |
| naive          |    0     |     0      | 100.00% |
| pi             | 39956380 |  32925342  | 82.40%  |
| qsort          |  200045  |   174888   | 87.42%  |
| queens         |  77116   |   56588    | 73.38%  |
| statement_test |   202    |    122     | 60.40%  |
| superloop      |  435027  |   408156   | 93.82%  |
| tak            |  60639   |   44755    | 73.81%  |

