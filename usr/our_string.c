#include "our_string.h"

// 从标准输入获取一行
bool read_line(char* str, int length)
{
    int c;
    int index = 0;
    while (1)
    {
        c = kernel_getchar();
        // 回车，结束输入
        if ('\n' == c)
        {
            // 确保字符串结尾是'\0'
            str[index] = '\0';
            return true;
        }
        // 回退，删除一个字符，将其从数组中删除，也从屏幕上消失
        else if ('\b' == c)
        {
            if (index)
            {
                // 原ZJUNIX代码只有index--，这时按下回车不就出bug了
                str[index--] = '\0';
                // 删除字符，重置光标
                kernel_putchar_at(' ', cursor.row, cursor.col - 1);
                cursor.col--;
                kernel_set_cursor();
            }
        }
        // 输入了EOF，现在应该还遇不到
        else if (-1 == c)
        {
            return false;
        }
        // 输入一般字符，存在字符串中并显示出来
        else
        {
            // length要减1，结尾给'\0'留个位置
            if (index < length - 1)
            {
                str[index++] = c;
                kernel_putchar(c);
            }
        }
    }
}