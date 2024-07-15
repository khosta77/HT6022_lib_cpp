# HT6022_lib_cpp

Библиотека на С++ для работы с осциллографом Hantek HT6022BE. За основу была взята библиотека для C от [rpm2003rpm](https://github.com/rpm2003rpm/HT6022_Driver/tree/master). Свяерялся с результатом от [openhantek](https://github.com/OpenHantek/OpenHantek6022).

Завернул в пространство имен `oscilloscopes` в дальнейшем возможно добавить другие осциллографы. И отдельное простаранство имен по производителя `hantek`.

## Запуск

Скопируйте файлы `ht6022lib.h` и `ht6022lib.cpp` в ваш проект, подключите библиотеку `libusb-1.0` в вашей системе сборки

## Пример

В примере ниже происходит считывание сигнала с двух каналов, так же заданы калибровочные значение. *у вас они могу быть другими*.

```C++
#include "ht6022lib.h"

int main()
{
    oscilloscopes::hantek::ht6022be osc;
    auto signal = osc.readFrame(oscilloscopes::hantek::HT6022_4KB, 0.914306640625, 9.122314453125);
    std::ofstream out_ch1("signal_CH1.txt"), out_ch2("signal_CH2.txt"); 

    for( auto it : signal.first )
        out_ch1 << ((int)it) << ' ';
    for( auto it : signal.second )
        out_ch2 << ((int)it) << ' ';

    out_ch1.close();
    out_ch2.close();
    return 0;
}
```

Графики сигналов:

![](img/signal.png)
