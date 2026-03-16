/*
 * 测试程序 - 用于Simple C Compiler
 * 这个程序演示了编译器支持的各种语法特性
 */

// 全局变量声明
int global_var;

// 计算阶乘的函数
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

// 计算斐波那契数列
int fibonacci(int n) {
    if (n <= 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }
    
    int a;
    int b;
    int temp;
    int i;
    
    a = 0;
    b = 1;
    i = 2;
    
    while (i <= n) {
        temp = a + b;
        a = b;
        b = temp;
        i = i + 1;
    }
    
    return b;
}

// 主函数
int main() {
    int x;
    int y;
    int result;
    
    // 变量赋值
    x = 10;
    y = 5;
    
    // 算术运算
    result = x + y;
    result = x - y;
    result = x * y;
    result = x / y;
    result = x % y;
    
    // 比较运算
    if (x > y) {
        result = 1;
    } else {
        result = 0;
    }
    
    // for循环示例
    int i;
    for (i = 0; i < 10; i = i + 1) {
        result = result + i;
    }
    
    // while循环示例
    i = 0;
    while (i < 5) {
        result = result * 2;
        i = i + 1;
    }
    
    // 函数调用
    result = factorial(5);
    result = fibonacci(10);
    
    return 0;
}
