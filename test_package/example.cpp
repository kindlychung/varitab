#include <iostream>
#include "varitab.h"

int main() {
    auto locale_string = "en_US.UTF-8";
    setlocale(LC_ALL, locale_string);
    std::locale loc(locale_string);
    std::wcout.imbue(loc);

    {
        VariadicTableWide<std::wstring, double, int, std::wstring> vt(
            {L"Name", L"Weight", L"Age", L"Brother"}, 1);
        vt.addRow({L"Cody", 180.2, 40, L"John"});
        vt.addRow({L"David", 175.3, 38, L"Andrew"});
        vt.addRow({L"Robert", 140.3, 27, L"Fande"});
        vt.print(std::wcout);
    }

    // More Data
    {
        VariadicTableWide<std::wstring, double, double, double, int> vt(
            {L"Section/部分", L"Col1/第1栏", L"Col2/第2栏", L"Col3/第3栏",
             L"Col4/第4栏"},
            1);

        vt.setColumnFormat({
            VariadicTableColumnFormat::AUTO,
            VariadicTableColumnFormat::SCIENTIFIC,
            VariadicTableColumnFormat::FIXED,
            VariadicTableColumnFormat::PERCENT,
            VariadicTableColumnFormat::FIXED,
        });

        vt.setColumnPrecision({1, 3, 3, 2, 2});

        vt.addRow({L"根目录", 0.4525, 0.051815, 0.05634, -1113});
        vt.addRow({L"应用::安装", 1.33, 0.037072, 0.037082, 234});
        vt.addRow({L"初始化", 2.551, 0.001548, 0.001551, 77461});
        vt.addRow({L"Python爬取抖音APP,竟然只需要十行代码", 0.001548, 0,
                   0.001548, 48017});
        vt.addRow({L"千万别做老板最不能容忍的三种人 z", 5.1e-05, 0.000192,
                   0.000243, -483461});
        vt.addRow({L"腾讯 阿里 华为的岗位薪资情况概述", 6.7e-05, 0.002233,
                   0.0023, 430061});
        vt.addRow({L"程序员晒出小学儿子满分作文《我的爸爸》，真实的让人心疼",
                   0.002051, 0, 0.002051, 6678});

        vt.print(std::wcout);
    }
}
