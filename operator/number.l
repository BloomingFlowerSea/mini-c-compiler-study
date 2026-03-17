%{
/* 第一部分：C 语言声明和头文件 */
#include <stdio.h>
%}

/* 第一部分：正则表达式宏定义 (完美还原你图片中的文法) */
DIGIT       [0-9]
DIGITS      {DIGIT}+
OPT_FRAC    (\.{DIGITS})?
OPT_EXP     (E[+-]?{DIGITS})?
NUMBER      {DIGITS}{OPT_FRAC}{OPT_EXP}

%%
  /* 第二部分：词法规则 (注意：左侧是正则，右侧是动作) */

{NUMBER}    { printf("[ACCEPT] 这是一个合法的数字: %s\n", yytext); }

[ \t\n\r]+  { /* 遇到空格、制表符、换行符直接忽略，不执行任何操作 */ }

.           { printf("[REJECT] 词法错误或非法字符: %s\n", yytext); }

%%
/* 第三部分：用户自定义的 C 代码 */

int main(int argc, char **argv) {
    printf("Lexer 已启动。请输入字符串 (输入 Ctrl+D 触发 EOF 结束程序):\n");
    // yylex() 是 flex 自动生成的核心词法分析函数
    yylex(); 
    return 0;
}

/* yywrap 函数用于处理多个输入文件的情况。
 * 返回 1 表示只有一个输入流（标准输入），读完就结束。*/
int yywrap() {
    return 1;
}